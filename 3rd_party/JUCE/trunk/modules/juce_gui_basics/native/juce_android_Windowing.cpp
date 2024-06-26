/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

} // (juce namespace)

extern juce::JUCEApplicationBase* juce_CreateApplication(); // (from START_JUCE_APPLICATION)

namespace juce
{

//==============================================================================
JUCE_JNI_CALLBACK (JUCE_ANDROID_ACTIVITY_CLASSNAME, launchApp, void, (JNIEnv* env, jobject activity,
                                                                      jstring appFile, jstring appDataDir))
{
    android.initialise (env, activity, appFile, appDataDir);

    DBG (SystemStats::getJUCEVersion());

    JUCEApplicationBase::createInstance = &juce_CreateApplication;

    initialiseJuce_GUI();

    JUCEApplication* app = dynamic_cast <JUCEApplication*> (JUCEApplicationBase::createInstance());
    if (! app->initialiseApp())
        exit (0);

    jassert (MessageManager::getInstance()->isThisTheMessageThread());
}

JUCE_JNI_CALLBACK (JUCE_ANDROID_ACTIVITY_CLASSNAME, suspendApp, void, (JNIEnv* env, jobject activity))
{
    if (JUCEApplicationBase* const app = JUCEApplicationBase::getInstance())
        app->suspended();
}

JUCE_JNI_CALLBACK (JUCE_ANDROID_ACTIVITY_CLASSNAME, resumeApp, void, (JNIEnv* env, jobject activity))
{
    if (JUCEApplicationBase* const app = JUCEApplicationBase::getInstance())
        app->resumed();
}

JUCE_JNI_CALLBACK (JUCE_ANDROID_ACTIVITY_CLASSNAME, quitApp, void, (JNIEnv* env, jobject activity))
{
    JUCEApplicationBase::appWillTerminateByForce();

    android.shutdown (env);
}

//==============================================================================
#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (drawBitmap,       "drawBitmap",    "([IIIFFIIZLandroid/graphics/Paint;)V") \
 METHOD (getClipBounds,    "getClipBounds", "()Landroid/graphics/Rect;")

DECLARE_JNI_CLASS (CanvasMinimal, "android/graphics/Canvas");
#undef JNI_CLASS_MEMBERS

//==============================================================================
#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (setViewName,   "setViewName",      "(Ljava/lang/String;)V") \
 METHOD (layout,        "layout",           "(IIII)V") \
 METHOD (getLeft,       "getLeft",          "()I") \
 METHOD (getTop,        "getTop",           "()I") \
 METHOD (getWidth,      "getWidth",         "()I") \
 METHOD (getHeight,     "getHeight",        "()I") \
 METHOD (getLocationOnScreen, "getLocationOnScreen", "([I)V") \
 METHOD (bringToFront,  "bringToFront",     "()V") \
 METHOD (requestFocus,  "requestFocus",     "()Z") \
 METHOD (setVisible,    "setVisible",       "(Z)V") \
 METHOD (isVisible,     "isVisible",        "()Z") \
 METHOD (hasFocus,      "hasFocus",         "()Z") \
 METHOD (invalidate,    "invalidate",       "(IIII)V") \
 METHOD (containsPoint, "containsPoint",    "(II)Z") \
 METHOD (showKeyboard,  "showKeyboard",     "(Z)V") \
 METHOD (createGLView,  "createGLView",     "()L" JUCE_ANDROID_ACTIVITY_CLASSPATH "$OpenGLView;") \

DECLARE_JNI_CLASS (ComponentPeerView, JUCE_ANDROID_ACTIVITY_CLASSPATH "$ComponentPeerView");
#undef JNI_CLASS_MEMBERS


//==============================================================================
class AndroidComponentPeer  : public ComponentPeer
{
public:
    AndroidComponentPeer (Component& comp, const int windowStyleFlags)
        : ComponentPeer (comp, windowStyleFlags),
          usingAndroidGraphics (false),
          fullScreen (false),
          sizeAllocated (0)
    {
        // NB: must not put this in the initialiser list, as it invokes a callback,
        // which will fail if the peer is only half-constructed.
        view = GlobalRef (android.activity.callObjectMethod (JuceAppActivity.createNewView,
                                                             component.isOpaque()));

        if (isFocused())
            handleFocusGain();
    }

    ~AndroidComponentPeer()
    {
        if (MessageManager::getInstance()->isThisTheMessageThread())
        {
            android.activity.callVoidMethod (JuceAppActivity.deleteView, view.get());
        }
        else
        {
            struct ViewDeleter  : public CallbackMessage
            {
                ViewDeleter (const GlobalRef& view_) : view (view_) {}

                void messageCallback() override
                {
                    android.activity.callVoidMethod (JuceAppActivity.deleteView, view.get());
                }

            private:
                GlobalRef view;
            };

            (new ViewDeleter (view))->post();
        }

        view.clear();
    }

    void* getNativeHandle() const override
    {
        return (void*) view.get();
    }

    void setVisible (bool shouldBeVisible) override
    {
        if (MessageManager::getInstance()->isThisTheMessageThread())
        {
            view.callVoidMethod (ComponentPeerView.setVisible, shouldBeVisible);
        }
        else
        {
            struct VisibilityChanger  : public CallbackMessage
            {
                VisibilityChanger (const GlobalRef& view_, bool shouldBeVisible_)
                    : view (view_), shouldBeVisible (shouldBeVisible_)
                {}

                void messageCallback() override
                {
                    view.callVoidMethod (ComponentPeerView.setVisible, shouldBeVisible);
                }

            private:
                GlobalRef view;
                bool shouldBeVisible;
            };

            (new VisibilityChanger (view, shouldBeVisible))->post();
        }
    }

    void setTitle (const String& title) override
    {
        view.callVoidMethod (ComponentPeerView.setViewName, javaString (title).get());
    }

    void setBounds (const Rectangle<int>& r, bool isNowFullScreen) override
    {
        if (MessageManager::getInstance()->isThisTheMessageThread())
        {
            fullScreen = isNowFullScreen;
            view.callVoidMethod (ComponentPeerView.layout,
                                 r.getX(), r.getY(), r.getRight(), r.getBottom());
        }
        else
        {
            class ViewMover  : public CallbackMessage
            {
            public:
                ViewMover (const GlobalRef& v, const Rectangle<int>& r)  : view (v), bounds (r) {}

                void messageCallback() override
                {
                    view.callVoidMethod (ComponentPeerView.layout,
                                         bounds.getX(), bounds.getY(), bounds.getRight(), bounds.getBottom());
                }

            private:
                GlobalRef view;
                Rectangle<int> bounds;
            };

            (new ViewMover (view, r))->post();
        }
    }

    Rectangle<int> getBounds() const override
    {
        return Rectangle<int> (view.callIntMethod (ComponentPeerView.getLeft),
                               view.callIntMethod (ComponentPeerView.getTop),
                               view.callIntMethod (ComponentPeerView.getWidth),
                               view.callIntMethod (ComponentPeerView.getHeight));
    }

    void handleScreenSizeChange()
    {
        ComponentPeer::handleScreenSizeChange();

        if (isFullScreen())
            setFullScreen (true);
    }

    Point<int> getScreenPosition() const
    {
        return Point<int> (view.callIntMethod (ComponentPeerView.getLeft),
                           view.callIntMethod (ComponentPeerView.getTop));
    }

    Point<int> localToGlobal (Point<int> relativePosition) override
    {
        return relativePosition + getScreenPosition();
    }

    Point<int> globalToLocal (Point<int> screenPosition) override
    {
        return screenPosition - getScreenPosition();
    }

    void setMinimised (bool shouldBeMinimised) override
    {
        // n/a
    }

    bool isMinimised() const override
    {
        return false;
    }

    void setFullScreen (bool shouldBeFullScreen) override
    {
        Rectangle<int> r (shouldBeFullScreen ? Desktop::getInstance().getDisplays().getMainDisplay().userArea
                                             : lastNonFullscreenBounds);

        if ((! shouldBeFullScreen) && r.isEmpty())
            r = getBounds();

        // (can't call the component's setBounds method because that'll reset our fullscreen flag)
        if (! r.isEmpty())
            setBounds (r, shouldBeFullScreen);

        component.repaint();
    }

    bool isFullScreen() const override
    {
        return fullScreen;
    }

    void setIcon (const Image& newIcon) override
    {
        // n/a
    }

    bool contains (Point<int> localPos, bool trueIfInAChildWindow) const override
    {
        return isPositiveAndBelow (localPos.x, component.getWidth())
            && isPositiveAndBelow (localPos.y, component.getHeight())
            && ((! trueIfInAChildWindow) || view.callBooleanMethod (ComponentPeerView.containsPoint,
                                                                    localPos.x, localPos.y));
    }

    BorderSize<int> getFrameSize() const override
    {
        // TODO
        return BorderSize<int>();
    }

    bool setAlwaysOnTop (bool alwaysOnTop) override
    {
        // TODO
        return false;
    }

    void toFront (bool makeActive) override
    {
        view.callVoidMethod (ComponentPeerView.bringToFront);

        if (makeActive)
            grabFocus();

        handleBroughtToFront();
    }

    void toBehind (ComponentPeer* other) override
    {
        // TODO
    }

    //==============================================================================
    void handleMouseDownCallback (int index, float x, float y, int64 time)
    {
        lastMousePos.setXY ((int) x, (int) y);
        currentModifiers = currentModifiers.withoutMouseButtons();
        handleMouseEvent (index, lastMousePos, currentModifiers, time);
        currentModifiers = currentModifiers.withoutMouseButtons().withFlags (ModifierKeys::leftButtonModifier);
        handleMouseEvent (index, lastMousePos, currentModifiers, time);
    }

    void handleMouseDragCallback (int index, float x, float y, int64 time)
    {
        lastMousePos.setXY ((int) x, (int) y);
        handleMouseEvent (index, lastMousePos, currentModifiers, time);
    }

    void handleMouseUpCallback (int index, float x, float y, int64 time)
    {
        lastMousePos.setXY ((int) x, (int) y);
        currentModifiers = currentModifiers.withoutMouseButtons();
        handleMouseEvent (index, lastMousePos, currentModifiers, time);
    }

    void handleKeyDownCallback (int k, int kc)
    {
        handleKeyPress (k, kc);
    }

    void handleKeyUpCallback (int k, int kc)
    {
    }

    //==============================================================================
    bool isFocused() const override
    {
        return view.callBooleanMethod (ComponentPeerView.hasFocus);
    }

    void grabFocus() override
    {
        view.callBooleanMethod (ComponentPeerView.requestFocus);
    }

    void handleFocusChangeCallback (bool hasFocus)
    {
        if (hasFocus)
            handleFocusGain();
        else
            handleFocusLoss();
    }

    void textInputRequired (const Point<int>&) override
    {
        view.callVoidMethod (ComponentPeerView.showKeyboard, true);
    }

    void dismissPendingTextInput() override
    {
        view.callVoidMethod (ComponentPeerView.showKeyboard, false);
     }

    //==============================================================================
    void handlePaintCallback (JNIEnv* env, jobject canvas)
    {
        jobject rect = env->CallObjectMethod (canvas, CanvasMinimal.getClipBounds);
        const int left   = env->GetIntField (rect, RectClass.left);
        const int top    = env->GetIntField (rect, RectClass.top);
        const int right  = env->GetIntField (rect, RectClass.right);
        const int bottom = env->GetIntField (rect, RectClass.bottom);
        env->DeleteLocalRef (rect);

        const Rectangle<int> clip (left, top, right - left, bottom - top);

        const int sizeNeeded = clip.getWidth() * clip.getHeight();
        if (sizeAllocated < sizeNeeded)
        {
            buffer.clear();
            sizeAllocated = sizeNeeded;
            buffer = GlobalRef (env->NewIntArray (sizeNeeded));
        }

        if (jint* dest = env->GetIntArrayElements ((jintArray) buffer.get(), 0))
        {
            {
                Image temp (new PreallocatedImage (clip.getWidth(), clip.getHeight(),
                                                   dest, ! component.isOpaque()));

                {
                    LowLevelGraphicsSoftwareRenderer g (temp);
                    g.setOrigin (-clip.getX(), -clip.getY());
                    handlePaint (g);
                }
            }

            env->ReleaseIntArrayElements ((jintArray) buffer.get(), dest, 0);

            env->CallVoidMethod (canvas, CanvasMinimal.drawBitmap, (jintArray) buffer.get(), 0, clip.getWidth(),
                                 (jfloat) clip.getX(), (jfloat) clip.getY(),
                                 clip.getWidth(), clip.getHeight(), true, (jobject) 0);
        }
    }

    void repaint (const Rectangle<int>& area) override
    {
        if (MessageManager::getInstance()->isThisTheMessageThread())
        {
            view.callVoidMethod (ComponentPeerView.invalidate, area.getX(), area.getY(), area.getRight(), area.getBottom());
        }
        else
        {
            struct ViewRepainter  : public CallbackMessage
            {
                ViewRepainter (const GlobalRef& view_, const Rectangle<int>& area_)
                    : view (view_), area (area_) {}

                void messageCallback() override
                {
                    view.callVoidMethod (ComponentPeerView.invalidate, area.getX(), area.getY(),
                                         area.getRight(), area.getBottom());
                }

            private:
                GlobalRef view;
                const Rectangle<int> area;
            };

            (new ViewRepainter (view, area))->post();
        }
    }

    void performAnyPendingRepaintsNow() override
    {
        // TODO
    }

    void setAlpha (float newAlpha) override
    {
        // TODO
    }

    StringArray getAvailableRenderingEngines() override
    {
        return StringArray ("Software Renderer");
    }

    //==============================================================================
    static AndroidComponentPeer* findPeerForJavaView (JNIEnv* env, jobject viewToFind)
    {
        for (int i = getNumPeers(); --i >= 0;)
        {
            AndroidComponentPeer* const ap = static_cast <AndroidComponentPeer*> (getPeer(i));
            jassert (dynamic_cast <AndroidComponentPeer*> (getPeer(i)) != nullptr);

            if (env->IsSameObject (ap->view.get(), viewToFind))
                return ap;
        }

        return nullptr;
    }

    static ModifierKeys currentModifiers;
    static Point<int> lastMousePos;

private:
    //==============================================================================
    GlobalRef view;
    GlobalRef buffer;
    bool usingAndroidGraphics, fullScreen;
    int sizeAllocated;

    class PreallocatedImage  : public ImagePixelData
    {
    public:
        PreallocatedImage (const int width_, const int height_, jint* data_, bool hasAlpha_)
            : ImagePixelData (Image::ARGB, width_, height_), data (data_), hasAlpha (hasAlpha_)
        {
            if (hasAlpha_)
                zeromem (data_, width * height * sizeof (jint));
        }

        ~PreallocatedImage()
        {
            if (hasAlpha)
            {
                PixelARGB* pix = (PixelARGB*) data;

                for (int i = width * height; --i >= 0;)
                {
                    pix->unpremultiply();
                    ++pix;
                }
            }
        }

        ImageType* createType() const override                      { return new SoftwareImageType(); }
        LowLevelGraphicsContext* createLowLevelContext() override   { return new LowLevelGraphicsSoftwareRenderer (Image (this)); }

        void initialiseBitmapData (Image::BitmapData& bm, int x, int y, Image::BitmapData::ReadWriteMode mode)
        {
            bm.lineStride = width * sizeof (jint);
            bm.pixelStride = sizeof (jint);
            bm.pixelFormat = Image::ARGB;
            bm.data = (uint8*) (data + x + y * width);
        }

        ImagePixelData* clone()
        {
            PreallocatedImage* s = new PreallocatedImage (width, height, 0, hasAlpha);
            s->allocatedData.malloc (sizeof (jint) * width * height);
            s->data = s->allocatedData;
            memcpy (s->data, data, sizeof (jint) * width * height);
            return s;
        }

    private:
        jint* data;
        HeapBlock<jint> allocatedData;
        bool hasAlpha;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PreallocatedImage)
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AndroidComponentPeer)
};

ModifierKeys AndroidComponentPeer::currentModifiers = 0;
Point<int> AndroidComponentPeer::lastMousePos;

//==============================================================================
#define JUCE_VIEW_CALLBACK(returnType, javaMethodName, params, juceMethodInvocation) \
  JUCE_JNI_CALLBACK (JUCE_JOIN_MACRO (JUCE_ANDROID_ACTIVITY_CLASSNAME, _00024ComponentPeerView), javaMethodName, returnType, params) \
  { \
      if (AndroidComponentPeer* const peer = AndroidComponentPeer::findPeerForJavaView (env, view)) \
          peer->juceMethodInvocation; \
  }

JUCE_VIEW_CALLBACK (void, handlePaint,      (JNIEnv* env, jobject view, jobject canvas),                          handlePaintCallback (env, canvas))
JUCE_VIEW_CALLBACK (void, handleMouseDown,  (JNIEnv* env, jobject view, jint i, jfloat x, jfloat y, jlong time),  handleMouseDownCallback (i, (float) x, (float) y, (int64) time))
JUCE_VIEW_CALLBACK (void, handleMouseDrag,  (JNIEnv* env, jobject view, jint i, jfloat x, jfloat y, jlong time),  handleMouseDragCallback (i, (float) x, (float) y, (int64) time))
JUCE_VIEW_CALLBACK (void, handleMouseUp,    (JNIEnv* env, jobject view, jint i, jfloat x, jfloat y, jlong time),  handleMouseUpCallback (i, (float) x, (float) y, (int64) time))
JUCE_VIEW_CALLBACK (void, viewSizeChanged,  (JNIEnv* env, jobject view),                                          handleMovedOrResized())
JUCE_VIEW_CALLBACK (void, focusChanged,     (JNIEnv* env, jobject view, jboolean hasFocus),                       handleFocusChangeCallback (hasFocus))
JUCE_VIEW_CALLBACK (void, handleKeyDown,    (JNIEnv* env, jobject view, jint k, jint kc),                         handleKeyDownCallback ((int) k, (int) kc))
JUCE_VIEW_CALLBACK (void, handleKeyUp,      (JNIEnv* env, jobject view, jint k, jint kc),                         handleKeyUpCallback ((int) k, (int) kc))

//==============================================================================
ComponentPeer* Component::createNewPeer (int styleFlags, void*)
{
    return new AndroidComponentPeer (*this, styleFlags);
}

jobject createOpenGLView (ComponentPeer* peer)
{
    jobject parentView = static_cast <jobject> (peer->getNativeHandle());
    return getEnv()->CallObjectMethod (parentView, ComponentPeerView.createGLView);
}

//==============================================================================
bool Desktop::canUseSemiTransparentWindows() noexcept
{
    return true;
}

double Desktop::getDefaultMasterScale()
{
    return 1.0;
}

Desktop::DisplayOrientation Desktop::getCurrentOrientation() const
{
    // TODO
    return upright;
}

bool Desktop::addMouseInputSource()
{
    mouseSources.add (new MouseInputSource (mouseSources.size(), false));
    return true;
}

Point<int> MouseInputSource::getCurrentRawMousePosition()
{
    return AndroidComponentPeer::lastMousePos;
}

void MouseInputSource::setRawMousePosition (Point<int>)
{
    // not needed
}

//==============================================================================
bool KeyPress::isKeyCurrentlyDown (const int keyCode)
{
    // TODO
    return false;
}

void ModifierKeys::updateCurrentModifiers() noexcept
{
    currentModifiers = AndroidComponentPeer::currentModifiers;
}

ModifierKeys ModifierKeys::getCurrentModifiersRealtime() noexcept
{
    return AndroidComponentPeer::currentModifiers;
}

//==============================================================================
// TODO
JUCE_API bool JUCE_CALLTYPE Process::isForegroundProcess() { return true; }
JUCE_API void JUCE_CALLTYPE Process::makeForegroundProcess() {}
JUCE_API void JUCE_CALLTYPE Process::hide() {}

//==============================================================================
void JUCE_CALLTYPE NativeMessageBox::showMessageBoxAsync (AlertWindow::AlertIconType iconType,
                                                          const String& title, const String& message,
                                                          Component* associatedComponent,
                                                          ModalComponentManager::Callback* callback)
{
    android.activity.callVoidMethod (JuceAppActivity.showMessageBox, javaString (title).get(),
                                     javaString (message).get(), (jlong) (pointer_sized_int) callback);
}

bool JUCE_CALLTYPE NativeMessageBox::showOkCancelBox (AlertWindow::AlertIconType iconType,
                                                      const String& title, const String& message,
                                                      Component* associatedComponent,
                                                      ModalComponentManager::Callback* callback)
{
    jassert (callback != nullptr); // on android, all alerts must be non-modal!!

    android.activity.callVoidMethod (JuceAppActivity.showOkCancelBox, javaString (title).get(),
                                     javaString (message).get(), (jlong) (pointer_sized_int) callback);
    return false;
}

int JUCE_CALLTYPE NativeMessageBox::showYesNoCancelBox (AlertWindow::AlertIconType iconType,
                                                        const String& title, const String& message,
                                                        Component* associatedComponent,
                                                        ModalComponentManager::Callback* callback)
{
    jassert (callback != nullptr); // on android, all alerts must be non-modal!!

    android.activity.callVoidMethod (JuceAppActivity.showYesNoCancelBox, javaString (title).get(),
                                     javaString (message).get(), (jlong) (pointer_sized_int) callback);
    return 0;
}

JUCE_JNI_CALLBACK (JUCE_ANDROID_ACTIVITY_CLASSNAME, alertDismissed, void, (JNIEnv* env, jobject activity,
                                                                           jlong callbackAsLong, jint result))
{
    if (ModalComponentManager::Callback* callback = (ModalComponentManager::Callback*) callbackAsLong)
        callback->modalStateFinished (result);
}

//==============================================================================
void Desktop::setScreenSaverEnabled (const bool isEnabled)
{
    // TODO
}

bool Desktop::isScreenSaverEnabled()
{
    return true;
}

//==============================================================================
void Desktop::setKioskComponent (Component* kioskModeComponent, bool enableOrDisable, bool allowMenusAndBars)
{
    // TODO
}

//==============================================================================
bool juce_areThereAnyAlwaysOnTopWindows()
{
    return false;
}

//==============================================================================
void Desktop::Displays::findDisplays (float masterScale)
{
    Display d;
    d.userArea = d.totalArea = Rectangle<int> (android.screenWidth,
                                               android.screenHeight) / masterScale;
    d.isMain = true;
    d.scale = masterScale;

    displays.add (d);
}

JUCE_JNI_CALLBACK (JUCE_ANDROID_ACTIVITY_CLASSNAME, setScreenSize, void, (JNIEnv* env, jobject activity,
                                                                          jint screenWidth, jint screenHeight,
                                                                          jint dpi))
{
    android.screenWidth = screenWidth;
    android.screenHeight = screenHeight;
    android.dpi = dpi;

    const_cast <Desktop::Displays&> (Desktop::getInstance().getDisplays()).refresh();
}

//==============================================================================
Image juce_createIconForFile (const File& file)
{
    return Image::null;
}

//==============================================================================
void* CustomMouseCursorInfo::create() const                                                     { return nullptr; }
void* MouseCursor::createStandardMouseCursor (const MouseCursor::StandardCursorType)            { return nullptr; }
void MouseCursor::deleteMouseCursor (void* const /*cursorHandle*/, const bool /*isStandard*/)   {}

//==============================================================================
void MouseCursor::showInWindow (ComponentPeer*) const   {}
void MouseCursor::showInAllWindows() const  {}

//==============================================================================
bool DragAndDropContainer::performExternalDragDropOfFiles (const StringArray& files, const bool canMove)
{
    return false;
}

bool DragAndDropContainer::performExternalDragDropOfText (const String& text)
{
    return false;
}

//==============================================================================
void LookAndFeel::playAlertSound()
{
}

//==============================================================================
void SystemClipboard::copyTextToClipboard (const String& text)
{
    const LocalRef<jstring> t (javaString (text));
    android.activity.callVoidMethod (JuceAppActivity.setClipboardContent, t.get());
}

String SystemClipboard::getTextFromClipboard()
{
    const LocalRef<jstring> text ((jstring) android.activity.callObjectMethod (JuceAppActivity.getClipboardContent));
    return juceString (text);
}

//==============================================================================
const int extendedKeyModifier       = 0x10000;

const int KeyPress::spaceKey        = ' ';
const int KeyPress::returnKey       = 66;
const int KeyPress::escapeKey       = 4;
const int KeyPress::backspaceKey    = 67;
const int KeyPress::leftKey         = extendedKeyModifier + 1;
const int KeyPress::rightKey        = extendedKeyModifier + 2;
const int KeyPress::upKey           = extendedKeyModifier + 3;
const int KeyPress::downKey         = extendedKeyModifier + 4;
const int KeyPress::pageUpKey       = extendedKeyModifier + 5;
const int KeyPress::pageDownKey     = extendedKeyModifier + 6;
const int KeyPress::endKey          = extendedKeyModifier + 7;
const int KeyPress::homeKey         = extendedKeyModifier + 8;
const int KeyPress::deleteKey       = extendedKeyModifier + 9;
const int KeyPress::insertKey       = -1;
const int KeyPress::tabKey          = 61;
const int KeyPress::F1Key           = extendedKeyModifier + 10;
const int KeyPress::F2Key           = extendedKeyModifier + 11;
const int KeyPress::F3Key           = extendedKeyModifier + 12;
const int KeyPress::F4Key           = extendedKeyModifier + 13;
const int KeyPress::F5Key           = extendedKeyModifier + 14;
const int KeyPress::F6Key           = extendedKeyModifier + 16;
const int KeyPress::F7Key           = extendedKeyModifier + 17;
const int KeyPress::F8Key           = extendedKeyModifier + 18;
const int KeyPress::F9Key           = extendedKeyModifier + 19;
const int KeyPress::F10Key          = extendedKeyModifier + 20;
const int KeyPress::F11Key          = extendedKeyModifier + 21;
const int KeyPress::F12Key          = extendedKeyModifier + 22;
const int KeyPress::F13Key          = extendedKeyModifier + 23;
const int KeyPress::F14Key          = extendedKeyModifier + 24;
const int KeyPress::F15Key          = extendedKeyModifier + 25;
const int KeyPress::F16Key          = extendedKeyModifier + 26;
const int KeyPress::numberPad0      = extendedKeyModifier + 27;
const int KeyPress::numberPad1      = extendedKeyModifier + 28;
const int KeyPress::numberPad2      = extendedKeyModifier + 29;
const int KeyPress::numberPad3      = extendedKeyModifier + 30;
const int KeyPress::numberPad4      = extendedKeyModifier + 31;
const int KeyPress::numberPad5      = extendedKeyModifier + 32;
const int KeyPress::numberPad6      = extendedKeyModifier + 33;
const int KeyPress::numberPad7      = extendedKeyModifier + 34;
const int KeyPress::numberPad8      = extendedKeyModifier + 35;
const int KeyPress::numberPad9      = extendedKeyModifier + 36;
const int KeyPress::numberPadAdd            = extendedKeyModifier + 37;
const int KeyPress::numberPadSubtract       = extendedKeyModifier + 38;
const int KeyPress::numberPadMultiply       = extendedKeyModifier + 39;
const int KeyPress::numberPadDivide         = extendedKeyModifier + 40;
const int KeyPress::numberPadSeparator      = extendedKeyModifier + 41;
const int KeyPress::numberPadDecimalPoint   = extendedKeyModifier + 42;
const int KeyPress::numberPadEquals         = extendedKeyModifier + 43;
const int KeyPress::numberPadDelete         = extendedKeyModifier + 44;
const int KeyPress::playKey         = extendedKeyModifier + 45;
const int KeyPress::stopKey         = extendedKeyModifier + 46;
const int KeyPress::fastForwardKey  = extendedKeyModifier + 47;
const int KeyPress::rewindKey       = extendedKeyModifier + 48;

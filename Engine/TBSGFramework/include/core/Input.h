#pragma once
#include <functional>

#include "memory/Containers.h"
#include "memory/String.h"

namespace tbsg {
    enum class InputAction
    {
        EPressed,
        EReleased,
        EBeingPressed
    };

    enum class InputType
    {
        None = 0x00, // No key was pressed
        EMouse_Left = 0x01, // Left mouse button
        EMouse_Right = 0x02, // Right mouse button
        Cancel = 0x03, // Cancel key
        EMouse_Middle = 0x04, // Middle mouse button
        EMouse_X1 = 0x05, // X1 mouse button
        EMouse_X2 = 0x06, // X2 mouse button
        // 0x07 is undefined
        Back = 0x08,
        Tab = 0x09,
        // 0x0A-0B are reserved
        Clear = 0x0c, // The CLEAR key
        Enter = 0x0d, // The Enter key
        // 0x0E-0F are undefined
        ShiftKey = 0x10, // The Shift key
        ControlKey = 0x11, // The Ctrl key
        AltKey = 0x12, // The Alt key
        Pause = 0x13, // The Pause key
        Capital = 0x14, // The Caps Lock key
        CapsLock = 0x14, // The Caps Lock key
        KanaMode = 0x15, // IMI Kana mode
        HanguelMode = 0x15, // IMI Hanguel mode (Use HangulMode)
        HangulMode = 0x15, // IMI Hangul mode
        // 0x16 is undefined
        JunjaMode = 0x17, // IMI Janja mode
        FinalMode = 0x18, // IMI Final mode
        HanjaMode = 0x19, // IMI Hanja mode
        KanjiMode = 0x19, // IMI Kanji mode
        // 0x1A is undefined
        Escape = 0x1b, // The ESC key
        IMEConvert = 0x1c, // IMI convert key
        IMINoConvert = 0x1d, // IMI noconvert key
        IMEAccept = 0x1e, // IMI accept key
        IMIModeChange = 0x1f, // IMI mode change key
        Space = 0x20, // The Space key
        Prior = 0x21, // The Page Up key
        PageUp = 0x21, // The Page Up key
        Next = 0x22, // The Page Down key
        PageDown = 0x22, // The Page Down key
        End = 0x23, // The End key
        Home = 0x24, // The Home key
        Left = 0x25, // The Left arrow key
        Up = 0x26, // The Up arrow key
        Right = 0x27, // The Right arrow key
        Down = 0x28, // The Down arrow key
        Select = 0x29, // The Select key
        Print = 0x2a, // The Print key
        Execute = 0x2b, // The Execute key
        PrintScreen = 0x2c, // The Print Screen key
        Snapshot = 0x2c, // The Print Screen key
        Insert = 0x2d, // The Insert key
        Delete = 0x2e, // The Delete key
        Help = 0x2f, // The Help key
        D0 = 0x30, // 0
        D1 = 0x31, // 1
        D2 = 0x32, // 2
        D3 = 0x33, // 3
        D4 = 0x34, // 4
        D5 = 0x35, // 5
        D6 = 0x36, // 6
        D7 = 0x37, // 7
        D8 = 0x38, // 8
        D9 = 0x39, // 9
        // 0x3A - 40 are undefined
        A = 0x41, // A
        B = 0x42, // B
        C = 0x43, // C
        D = 0x44, // D
        E = 0x45, // E
        F = 0x46, // F
        G = 0x47, // G
        H = 0x48, // H
        I = 0x49, // I
        J = 0x4a, // J
        K = 0x4b, // K
        L = 0x4c, // L
        M = 0x4d, // M
        N = 0x4e, // N
        O = 0x4f, // O
        P = 0x50, // P
        Q = 0x51, // Q
        R = 0x52, // R
        S = 0x53, // S
        T = 0x54, // T
        U = 0x55, // U
        V = 0x56, // V
        W = 0x57, // W
        X = 0x58, // X
        Y = 0x59, // Y
        Z = 0x5a, // Z
        LWin = 0x5b, // Left Windows key
        RWin = 0x5c, // Right Windows key
        Apps = 0x5d, // Apps key
        // 0x5E is reserved
        Sleep = 0x5f, // The Sleep key
        NumPad0 = 0x60, // The Numeric keypad 0 key
        NumPad1 = 0x61, // The Numeric keypad 1 key
        NumPad2 = 0x62, // The Numeric keypad 2 key
        NumPad3 = 0x63, // The Numeric keypad 3 key
        NumPad4 = 0x64, // The Numeric keypad 4 key
        NumPad5 = 0x65, // The Numeric keypad 5 key
        NumPad6 = 0x66, // The Numeric keypad 6 key
        NumPad7 = 0x67, // The Numeric keypad 7 key
        NumPad8 = 0x68, // The Numeric keypad 8 key
        NumPad9 = 0x69, // The Numeric keypad 9 key
        Multiply = 0x6a, // The Multiply key
        Add = 0x6b, // The Add key
        Separator = 0x6c, // The Separator key
        Subtract = 0x6d, // The Subtract key
        Decimal = 0x6e, // The Decimal key
        Divide = 0x6f, // The Divide key
        F1 = 0x70, // The F1 key
        F2 = 0x71, // The F2 key
        F3 = 0x72, // The F3 key
        F4 = 0x73, // The F4 key
        F5 = 0x74, // The F5 key
        F6 = 0x75, // The F6 key
        F7 = 0x76, // The F7 key
        F8 = 0x77, // The F8 key
        F9 = 0x78, // The F9 key
        F10 = 0x79, // The F10 key
        F11 = 0x7a, // The F11 key
        F12 = 0x7b, // The F12 key
        F13 = 0x7c, // The F13 key
        F14 = 0x7d, // The F14 key
        F15 = 0x7e, // The F15 key
        F16 = 0x7f, // The F16 key
        F17 = 0x80, // The F17 key
        F18 = 0x81, // The F18 key
        F19 = 0x82, // The F19 key
        F20 = 0x83, // The F20 key
        F21 = 0x84, // The F21 key
        F22 = 0x85, // The F22 key
        F23 = 0x86, // The F23 key
        F24 = 0x87, // The F24 key
        // 0x88 - 0x8f are unassigned
        NumLock = 0x90, // The Num Lock key
        Scroll = 0x91, // The Scroll Lock key
        // 0x92 - 96 are OEM specific
        // 0x97 - 9f are unassigned
        LShiftKey = 0xa0, // The Left Shift key
        RShiftKey = 0xa1, // The Right Shift key
        LControlKey = 0xa2, // The Left Control key
        RControlKey = 0xa3, // The Right Control key
        LMenu = 0xa4, // The Left Alt key
        RMenu = 0xa5, // The Right Alt key
        BrowserBack = 0xa6, // The Browser Back key
        BrowserForward = 0xa7, // The Browser Forward key
        BrowserRefresh = 0xa8, // The Browser Refresh key
        BrowserStop = 0xa9, // The Browser Stop key
        BrowserSearch = 0xaa, // The Browser Search key
        BrowserFavorites = 0xab, // The Browser Favorites key
        BrowserHome = 0xac, // The Browser Home key
        VolumeMute = 0xad, // The Volume Mute key
        VolumeDown = 0xae, // The Volume Down key
        VolumeUp = 0xaf, // The Volume Up key
        MediaNextTrack = 0xb0, // The Next Track key
        MediaPreviousTrack = 0xb1, // The Previous Track key
        MediaStop = 0xb2, // The Stop Media key
        MediaPlayPause = 0xb3, // The Play/Pause Media key
        LaunchMail = 0xb4, // The Start Mail key
        SelectMedia = 0xb5, // The Select Media key
        LaunchApplication1 = 0xb6, // The Launch Application 1 key.
        LaunchApplication2 = 0xb7, // The Launch Application 2 key.
        // 0xB8 - B9 are reserved
        OemSemicolon = 0xba, // Used for miscellaneous characters; it can vary by keyboard.  For the US standard keyboard, the ';:' key
        Oem1 = 0xba, // Used for miscellaneous characters; it can vary by keyboard.  For the US standard keyboard, the ';:' key
        OemPlus = 0xbb, // For any country/region, the '+' key
        OemComma = 0xbc, // For any country/region, the ',' key
        OemMinus = 0xbd, // For any country/region, the '-' key
        OemPeriod = 0xbe, // For any country/region, the '.' key
        OemQuestion = 0xbf, // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '/?' key
        Oem2 = 0xbf, // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '/?' key
        OemTilde = 0xc0, // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '`~' key
        Oem3 = 0xc0, // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '`~' key
        // 0xC1 - D7 are reserved
        // 0xD8 - DA are unassigned
        OemOpenBrackets = 0xdb, // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '[{' key
        Oem4 = 0xdb, // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '[{' key
        OemPipe = 0xdc, // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '\|' key
        Oem5 = 0xdc, // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '\|' key
        OemCloseBrackets = 0xdd, // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the ']}' key
        Oem6 = 0xdd, // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the ']}' key
        OemQuotes = 0xde, // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the 'single-quote/double-quote' key
        Oem7 = 0xde, // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the 'single-quote/double-quote' key
        Oem8 = 0xdf, // Used for miscellaneous characters; it can vary by keyboard.
        // 0xE0 is reserved
        // 0xE1 is OEM specific
        OemBackslash = 0xe2, // Either the angle bracket key or the backslash key on the RT 102-key keyboard
        Oem102 = 0xe2, // Either the angle bracket key or the backslash key on the RT 102-key keyboard
        // 0xE3 - E4 OEM specific
        ProcessKey = 0xe5, // IME Process key
        // 0xE6 is OEM specific
        Packet = 0xe7, // Used to pass Unicode characters as if they were keystrokes. The Packet key value is the low word of a 32-bit virtual-key value used for non-keyboard input methods.
        // 0xE8 is unassigned
        // 0xE9 - F5 OEM specific
        Attn = 0xf6, // The Attn key
        CrSel = 0xf7, // The CrSel key
        ExSel = 0xf8, // The ExSel key
        EraseEof = 0xf9, // The Erase EOF key
        Play = 0xfa, // The Play key
        Zoom = 0xfb, // The Zoom key
        NoName = 0xfc, // Reserved
        Pa1 = 0xfd, // The PA1 key
        OemClear = 0xfe, // The Clear key

        EKeyboard_Last = 276,

        EMouse_Wheel,

        EGamepad_Left_Thumb_X,
        EGamepad_Left_Thumb_Y,
        EGamepad_Left_Trigger,
        EGamepad_Left_Shoulder,
        EGamepad_Left_Thumb_Button,

        EGamepad_Right_Thumb_X,
        EGamepad_Right_Thumb_Y,
        EGamepad_Right_Trigger,
        EGamepad_Right_Shoulder,
        EGamepad_Right_Thumb_Button,

        EGamepad_DPad_Up,
        EGamepad_DPad_Down,
        EGamepad_DPad_Left,
        EGamepad_DPad_Right,

        EGamepad_Face_Button_Up,
        EGamepad_Face_Button_Down,
        EGamepad_Face_Button_Left,
        EGamepad_Face_Button_Right,

        EGamepad_Special_Left,
        EGamepad_Special_Right,


        EInput_Last
    };


    struct Binder
    {
        Binder(void* target, ptl::string name)
        {
            m_Target = target;
            m_Name = name;
        }

        void* m_Target = nullptr;
        ptl::string m_Name;
    };


    struct BinderInputAction : Binder
    {
        template<typename T, typename F>
        BinderInputAction(F listener, ptl::string name, T target) : Binder(target, name)
        {
            m_Listener = std::bind(listener, target, std::placeholders::_1);
        }

        bool Dispatch(InputAction action) const
        {
            return m_Listener(action);
        }

        std::function<bool(InputAction)> m_Listener;

        bool operator==(const BinderInputAction& rhs) const noexcept
        {
            return m_Target == rhs.m_Target && m_Name.compare(rhs.m_Name) == 0;
        }
    };

    struct BinderAxis : Binder {

        template<typename T, typename F>
        BinderAxis(F listener, ptl::string name, T target) : Binder(target, name)
        {
            m_Listener = std::bind(listener, target, std::placeholders::_1);
        }

        bool Dispatch(float val) const
        {
            return m_Listener(val);
        }

        std::function<bool(float)> m_Listener;

        bool operator==(const BinderAxis& rhs) const noexcept
        {
            return m_Target == rhs.m_Target && m_Name.compare(rhs.m_Name) == 0;
        }
    };

	struct BinderCharacter : Binder
	{

		template<typename T, typename F>
		BinderCharacter(F listener, ptl::string name, T target) : Binder(target, name)
		{
			m_Listener = std::bind(listener, target, std::placeholders::_1);
		}

		bool Dispatch(char val) const
		{
			return m_Listener(val);
		}

		std::function<bool(char)> m_Listener;

		bool operator==(const BinderCharacter& rhs) const noexcept
		{
			return m_Target == rhs.m_Target && m_Name.compare(rhs.m_Name) == 0;
		}
	};

	struct BinderTextDialog : Binder
	{

		template<typename T, typename F>
		BinderTextDialog(F listener, ptl::string name, T target) : Binder(target, name)
		{
			m_Listener = std::bind(listener, target, std::placeholders::_1);
		}

		bool Dispatch(ptl::string val) const
		{
			return m_Listener(val);
		}

		std::function<bool(ptl::string)> m_Listener;

		bool operator==(const BinderTextDialog& rhs) const noexcept
		{
			return m_Target == rhs.m_Target && m_Name.compare(rhs.m_Name) == 0;
		}
	};


#define BINDACTION(X, Y) tbsg::BinderInputAction(X, #X, Y)
#define BINDAXIS(X, Y) tbsg::BinderAxis(X, #X, Y)
#define BINDCHARACTER(X, Y) tbsg::BinderCharacter(X, #X, Y)
#define BINDTEXTDIALOG(X, Y) tbsg::BinderTextDialog(X, #X, Y)

    class Input
    {
    public:
        Input(int refWinSizeX, int refWinSizeY);
        ~Input();

        void Update();

        void AddListener(BinderAxis listener, InputType inputType);
        void AddListener(BinderInputAction listener, InputType inputType);
		void AddListener(BinderCharacter listener);
		void AddListener(BinderTextDialog listener);

        void PushListener(BinderAxis listener, InputType inputType);
        void PushListener(BinderInputAction listener, InputType inputType);
		void PushListener(BinderCharacter listener);
		void PushListener(BinderTextDialog listener);

        void RemoveListener(BinderAxis listener, InputType inputType);
        void RemoveListener(BinderInputAction listener, InputType inputType);
		void RemoveListener(BinderCharacter listener);
		void RemoveListener(BinderTextDialog listener);

        void RemoveListener(void* instance, InputType inputType);
        void RemoveListener(void* instance);

        void SetDeadZone(float x, float y) noexcept;
        void GetDeadZone(float* x, float* y) noexcept;

        int GetCursorX();
        int GetCursorY();

        float GetControllerX();
        float GetControllerY();

        void GetCursorPosition(int& x, int& y) noexcept;
        void SetCursorPosition(int x, int y) const noexcept;


        float GetControllerSpeed() const noexcept;
        void SetControllerSpeed(float controllerSpeed) noexcept;
#ifdef _WIN32
        void OnWndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif

		static Input* m_Instance;
		void OpenInputDialog(bool obsecure = false, ptl::string currentText = "");
    private:
        void HandleController();
        void HandleKeyboard();
        void HandleMouse();
		void HandleKeyboardDialog();

        bool HasInterestCross(InputType inputType);

        void FireEvents(float value, InputType inputType);
        void FireEvents(InputAction action, InputType inputType);

        void FireEventsCross(float value, InputType inputType);
        void FireEventsCross(InputAction action, InputType inputType);

        void SetTypeDown(InputType type);
        void SetTypeUp(InputType type);

        bool SetControllerX(float x) noexcept;
        bool SetControllerY(float y) noexcept;

        float m_DeadzoneX = 0.07f;
        float m_DeadzoneY = 0.07f;

        ptl::map<InputType, ptl::vector<BinderAxis>> m_AxisListeners;
        ptl::map<InputType, ptl::vector<BinderInputAction>> m_ActionListeners;
		ptl::vector<BinderCharacter> characterListeners;
		ptl::vector<BinderTextDialog> textDialogListeners;

        bool m_BeingPressed[int(InputType::EInput_Last)] = {};

        float m_ControllerSpeed = 1.5f;
        float m_JoyX = 0.0f;
        float m_JoyY = 0.0f;
        int m_RefWinSizeX, m_RefWinSizeY;
		bool dialogOpened{};
		ptl::wstring dialogTextBuffer;

#ifdef PLATFORM_WINDOWS
        ptl::map<WORD, InputType> m_ControllerMapping;
#endif

#ifdef PLATFORM_PS
		ptl::map<unsigned int, InputType> m_ControllerMapping;

		int32_t m_handle;

#endif
    };
}

#pragma once
#include <sol.hpp>
#include "memory/String.h"
#include "memory/Containers.h"

namespace tbsg
{
    namespace scripting
    {
        /**
         * \ingroup Scripting
         */
        class LuaContext
        {
        public:
			~LuaContext();

            void Initialize();
            void Deallocate();
            sol::state& GetState();
            lua_State& GetLuaState();
            void OpenLib(sol::lib& lib);

            //// Function Helper:
            sol::function GetUnsafeFunction(const char*);


            /**
             * \brief ask for a function
             * \param func 
             * \ingroup Scripting
             */
            void GetFunction(sol::protected_function& func, const char*);
            /**
             * \brief  ask for a function
             * \param state 
             * \param func 
             * \ingroup Scripting
             */
            void GetFunction(sol::state& state, sol::protected_function& func, const char*);

            /**
             * \brief 
             * \return
             * \ingroup Scripting
             */
            sol::protected_function GetFunction(const char*);
            /**
             * \brief 
             * \param state 
             * \return 
             * \ingroup Scripting
             */
            sol::protected_function GetFunction(sol::state& state, const char*);

            /**
             * \brief 
             * \tparam T 
             * \tparam Args 
             * \param a_definition 
             * \param function 
             * \ingroup Scripting
             * \return 
             */
            template<typename T,typename ...Args>
            sol::function& GetUnsafeFunction(const char* a_definition, std::function<T(Args...)> function);

            /**
             * \brief 
             * \tparam T 
             * \tparam Args 
             * \param a_SolFunction 
             * \param args 
             * \ingroup Scripting
             * \return 
             */
            template<typename T,typename ...Args>
            T& GetFunctionResult(sol::protected_function a_SolFunction, Args...args);

            template<typename T, typename ...Args>
            void SetFunction(const char* a_Declaration, T);

            /**
             * \brief 
             * \tparam T 
             * \tparam Args 
             * \param a_Declaration 
             * \param a_Function 
             * \ingroup Scripting
             */
            template<typename T, typename ...Args>
            void SetFunction(const char* a_Declaration, std::function<T(Args...)> a_Function);

            /**
             * \brief 
             * \tparam T 
             * \param a_Declaration 
             * \ingroup Scripting
             */
            template<typename T>
            void SetType(const char* a_Declaration);

            //// Execution Helper:
            sol::unsafe_function_result RunUnsafeScript(const ptl::string&);
            /**
             * \brief 
             * \ingroup Scripting
             * \return 
             */
            sol::protected_function_result RunScript(const ptl::string&);
            sol::protected_function_result RunScript(sol::state& state, const ptl::string&);
            sol::protected_function_result LoadAndRunScriot(const ptl::string&);
            bool IsValid(sol::unsafe_function_result&);
            bool IsValid(sol::safe_function_result&);
            ptl::string GetErrorMessage(sol::safe_function_result&);
            sol::call_status GetCallStatus(sol::unsafe_function_result&);
            sol::call_status GetCallStatus(sol::safe_function_result&);

            bool GetIsInitialized(){ return m_IsInitialized; }
        private:
            sol::state m_lua = nullptr;
            lua_State* m_luaState = nullptr;
            ptl::vector<char> m_openedLibs;
            bool m_IsInitialized = false;
        };

        template <typename T, typename ... Args>
        sol::function& LuaContext::GetUnsafeFunction(const char* a_definition, std::function<T(Args...)> function)
        {
            sol::state_view lua = m_luaState;
            sol::object function_obj = lua[a_definition];
            if (function_obj.get_type() == sol::type::function && function_obj.is<std::function<T(Args...)>>())
            {
                sol::function func = lua[a_definition];
                return func;
            }
            sol::function func;
            return func;
        }

        template <typename T, typename ... Args>
        T& LuaContext::GetFunctionResult(sol::protected_function a_SolFunction, Args...args)
        {
            return a_SolFunction(args...);
        }

        // template<typename T, typename ...Args>
        // void SetFunction(const char* a_Declaration, T)
        // {
        //     sol::state_view lua = m_luaState;
        //     lua.set_function(a_Declaration, T);
        // }

        template <typename T, typename...Args>
        void LuaContext::SetFunction(const char* a_Declaration, std::function<T(Args...)> a_Function)
        {
            sol::state_view lua = m_luaState;
            lua.set_function(a_Declaration, a_Function);
        }

        template<typename T>
        void LuaContext::SetType(const char* a_Declaration)
        {
           sol::state& lua = GetState();
           lua.new_usertype<T>(a_Declaration, "new", sol::no_constructor);
        }
    }
}

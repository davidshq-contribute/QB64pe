'
' Minimal Compiler Context for Component Testing
'
' Provides minimal compiler initialization for testing components.
' Initializes only the required global state needed for components to run,
' without requiring full compiler initialization.
'

'$INCLUDE:'test_state_manager_declarations.bi'
'$INCLUDE:'../../source/utilities/include_provider.bi'

' Minimal compiler context structure
TYPE MinimalCompilerContext
    initialized AS LONG
    includeProviderInitialized AS LONG
    stateContext AS TestStateContext
END TYPE

' Initialize minimal compiler context
' context: Context to initialize
' components$: Comma-separated list of components to initialize ("hash,type,const" or "all")
SUB TestCompilerContext_Init (context AS MinimalCompilerContext, components$)
    ' Initialize include provider if not already done
    IF NOT context.includeProviderInitialized THEN
        IncludeProvider_Init
        context.includeProviderInitialized = -1
    END IF
    
    ' Initialize component state
    IF components$ = "" THEN components$ = "all"
    TestState_Init context.stateContext, components$
    
    context.initialized = -1
END SUB

' Clean up minimal compiler context
SUB TestCompilerContext_Cleanup (context AS MinimalCompilerContext)
    IF context.initialized THEN
        TestState_Cleanup context.stateContext
        context.initialized = 0
    END IF
END SUB

' Reset context to uninitialized state
SUB TestCompilerContext_Reset (context AS MinimalCompilerContext)
    TestState_Reset context.stateContext
    context.initialized = 0
    context.includeProviderInitialized = 0
END SUB

' Check if context is initialized
FUNCTION TestCompilerContext_IsInitialized& (context AS MinimalCompilerContext)
    TestCompilerContext_IsInitialized& = context.initialized
END FUNCTION

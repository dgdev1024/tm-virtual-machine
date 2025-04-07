/**
 * @file  Main.c
 */

#include <SDL2/SDL.h>
#include <TOMBOY/Tomboy.h>

// Constants ///////////////////////////////////////////////////////////////////////////////////////

#define TOMBOY_AUDIO_SAMPLE_SIZE            2048
#define TOMBOY_AUDIO_SAMPLE_QUEUED_MINIMUM  65536

// Static Members //////////////////////////////////////////////////////////////////////////////////

static TOMBOY_Program*      s_Program = NULL;
static TOMBOY_Engine*       s_Engine  = NULL;
static SDL_Window*          s_Window  = NULL;
static SDL_Renderer*        s_Renderer = NULL;
static SDL_Texture*         s_RenderTarget = NULL;
static SDL_AudioDeviceID    s_AudioDevice = 0;
static float*               s_AudioBuffer = NULL;
static size_t               s_AudioCursor = 0;

// Private Functions - Input Handling //////////////////////////////////////////////////////////////

static void TOMBOY_OnKeyDown (const SDL_KeyboardEvent* p_Event)
{
    // Key Controls:
    // - ESCAPE: Exit Application
    // - UP, W: D-Pad Up
    // - DOWN, S: D-Pad Down
    // - LEFT, A: D-Pad Left
    // - RIGHT, D: D-Pad Right
    // - Z, J: A Button
    // - X, K: B Button
    // - SPACE, ENTER: Start Button
    // - Left SHIFT, Right SHIFT: Select Button

    TOMBOY_Joypad* l_Joypad = TOMBOY_GetJoypad(s_Engine);
    switch (p_Event->keysym.sym)
    {
        case SDLK_ESCAPE:       exit(EXIT_SUCCESS); break;
        case SDLK_UP:
        case SDLK_w:            TOMBOY_PressButton(l_Joypad, TOMBOY_JB_UP); break;
        case SDLK_DOWN:
        case SDLK_s:            TOMBOY_PressButton(l_Joypad, TOMBOY_JB_DOWN); break;
        case SDLK_LEFT:
        case SDLK_a:            TOMBOY_PressButton(l_Joypad, TOMBOY_JB_LEFT); break;
        case SDLK_RIGHT:
        case SDLK_d:            TOMBOY_PressButton(l_Joypad, TOMBOY_JB_RIGHT); break;
        case SDLK_z:
        case SDLK_j:            TOMBOY_PressButton(l_Joypad, TOMBOY_JB_A); break;
        case SDLK_x:
        case SDLK_k:            TOMBOY_PressButton(l_Joypad, TOMBOY_JB_B); break;
        case SDLK_SPACE:
        case SDLK_RETURN:       TOMBOY_PressButton(l_Joypad, TOMBOY_JB_START); break;
        case SDLK_LSHIFT:
        case SDLK_RSHIFT:       TOMBOY_PressButton(l_Joypad, TOMBOY_JB_SELECT); break;
        default:                break;
    }
}

static void TOMBOY_OnKeyUp (const SDL_KeyboardEvent* p_Event)
{
    TOMBOY_Joypad* l_Joypad = TOMBOY_GetJoypad(s_Engine);
    switch (p_Event->keysym.sym)
    {
        case SDLK_UP:
        case SDLK_w:            TOMBOY_ReleaseButton(l_Joypad, TOMBOY_JB_UP); break;
        case SDLK_DOWN:
        case SDLK_s:            TOMBOY_ReleaseButton(l_Joypad, TOMBOY_JB_DOWN); break;
        case SDLK_LEFT:
        case SDLK_a:            TOMBOY_ReleaseButton(l_Joypad, TOMBOY_JB_LEFT); break;
        case SDLK_RIGHT:
        case SDLK_d:            TOMBOY_ReleaseButton(l_Joypad, TOMBOY_JB_RIGHT); break;
        case SDLK_z:
        case SDLK_j:            TOMBOY_ReleaseButton(l_Joypad, TOMBOY_JB_A); break;
        case SDLK_x:
        case SDLK_k:            TOMBOY_ReleaseButton(l_Joypad, TOMBOY_JB_B); break;
        case SDLK_SPACE:
        case SDLK_RETURN:       TOMBOY_ReleaseButton(l_Joypad, TOMBOY_JB_START); break;
        case SDLK_LSHIFT:
        case SDLK_RSHIFT:       TOMBOY_ReleaseButton(l_Joypad, TOMBOY_JB_SELECT); break;
        default:                break;
    }
}

static void TOMBOY_OnControllerButtonDown (const SDL_ControllerButtonEvent* p_Event)
{
    TOMBOY_Joypad* l_Joypad = TOMBOY_GetJoypad(s_Engine);
    switch (p_Event->button)
    {
        case SDL_CONTROLLER_BUTTON_A:           TOMBOY_PressButton(l_Joypad, TOMBOY_JB_A); break;
        case SDL_CONTROLLER_BUTTON_B:           TOMBOY_PressButton(l_Joypad, TOMBOY_JB_B); break;
        case SDL_CONTROLLER_BUTTON_START:       TOMBOY_PressButton(l_Joypad, TOMBOY_JB_START); break;
        case SDL_CONTROLLER_BUTTON_BACK:        TOMBOY_PressButton(l_Joypad, TOMBOY_JB_SELECT); break;
        case SDL_CONTROLLER_BUTTON_DPAD_UP:     TOMBOY_PressButton(l_Joypad, TOMBOY_JB_UP); break;
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:   TOMBOY_PressButton(l_Joypad, TOMBOY_JB_DOWN); break;
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT:   TOMBOY_PressButton(l_Joypad, TOMBOY_JB_LEFT); break;
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:  TOMBOY_PressButton(l_Joypad, TOMBOY_JB_RIGHT); break;
        default:                                break;
    }
}

static void TOMBOY_OnControllerButtonUp (const SDL_ControllerButtonEvent* p_Event)
{
    TOMBOY_Joypad* l_Joypad = TOMBOY_GetJoypad(s_Engine);
    switch (p_Event->button)
    {
        case SDL_CONTROLLER_BUTTON_A:           TOMBOY_ReleaseButton(l_Joypad, TOMBOY_JB_A); break;
        case SDL_CONTROLLER_BUTTON_B:           TOMBOY_ReleaseButton(l_Joypad, TOMBOY_JB_B); break;
        case SDL_CONTROLLER_BUTTON_START:       TOMBOY_ReleaseButton(l_Joypad, TOMBOY_JB_START); break;
        case SDL_CONTROLLER_BUTTON_BACK:        TOMBOY_ReleaseButton(l_Joypad, TOMBOY_JB_SELECT); break;
        case SDL_CONTROLLER_BUTTON_DPAD_UP:     TOMBOY_ReleaseButton(l_Joypad, TOMBOY_JB_UP); break;
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:   TOMBOY_ReleaseButton(l_Joypad, TOMBOY_JB_DOWN); break;
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT:   TOMBOY_ReleaseButton(l_Joypad, TOMBOY_JB_LEFT); break;
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:  TOMBOY_ReleaseButton(l_Joypad, TOMBOY_JB_RIGHT); break;
        default:                                break;
    }
}

// Private Functions - Application Loop ////////////////////////////////////////////////////////////

static void TOMBOY_HandleEvents ()
{
    SDL_Event l_Event;
    while (SDL_PollEvent(&l_Event))
    {
        switch (l_Event.type)
        {
            case SDL_QUIT:                  exit(EXIT_SUCCESS); break;
            case SDL_KEYDOWN:               TOMBOY_OnKeyDown(&l_Event.key); break;
            case SDL_KEYUP:                 TOMBOY_OnKeyUp(&l_Event.key); break;
            case SDL_CONTROLLERBUTTONDOWN:  TOMBOY_OnControllerButtonDown(&l_Event.cbutton); break;
            case SDL_CONTROLLERBUTTONUP:    TOMBOY_OnControllerButtonUp(&l_Event.cbutton); break;
            default:                        break;
        }
    }
}

static void TOMBOY_Update ()
{
    uint32_t l_QueuedAudioSize = SDL_GetQueuedAudioSize(s_AudioDevice);
    if (l_QueuedAudioSize < TOMBOY_AUDIO_SAMPLE_QUEUED_MINIMUM)
    {
        SDL_QueueAudio(s_AudioDevice, s_AudioBuffer, s_AudioCursor * sizeof(float));
        s_AudioCursor = 0;
    }
}

static void TOMBOY_Render (TOMBOY_PPU* p_PPU)
{
    uint32_t*       l_Pixels = NULL;
    int32_t         l_Pitch = 0;

    // Lock the texture for writing.
    SDL_LockTexture(s_RenderTarget, NULL, (void**) &l_Pixels, &l_Pitch);

    // Get the screen buffer from the PPU. Copy its data into the pixels buffer.
    SDL_memcpy(l_Pixels, TOMBOY_GetScreenBuffer(p_PPU), TOMBOY_PPU_SCREEN_BUFFER_SIZE);

    // Unlock the texture, then render it.
    SDL_UnlockTexture(s_RenderTarget);
    SDL_RenderCopy(s_Renderer, s_RenderTarget, NULL, NULL);
    SDL_RenderPresent(s_Renderer);
}

// Private Functions - Frame and Audio Mix Callbacks ///////////////////////////////////////////////

static void TOMBOY_OnFrameRendered (TOMBOY_PPU* p_PPU)
{
    TOMBOY_HandleEvents();
    TOMBOY_Update();
    TOMBOY_Render(p_PPU);

    SDL_Delay(16); // Simulate a frame delay for 60 FPS
}

static void TOMBOY_OnAudioMix (const TOMBOY_AudioSample* p_Sample)
{
    if (s_AudioCursor + 2 < TOMBOY_AUDIO_SAMPLE_SIZE)
    {
        s_AudioBuffer[s_AudioCursor++] = p_Sample->m_Left;
        s_AudioBuffer[s_AudioCursor++] = p_Sample->m_Right;
    }
}

// Private Functions - Start and Exit //////////////////////////////////////////////////////////////

static void TOMBOY_AtStart (const char* p_ProgramFilename)
{
    // Initialize SDL.
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    // Create and load the program.
    s_Program = TOMBOY_CreateProgram(p_ProgramFilename);
    if (s_Program == NULL)
    {
        fprintf(stderr, "Failed to create program from file '%s'.\n", p_ProgramFilename);
        exit(EXIT_FAILURE);
    }

    // Create the engine instance.
    s_Engine = TOMBOY_CreateEngine(s_Program);
    if (s_Engine == NULL)
    {
        fprintf(stderr, "Failed to create engine instance.\n");
        TOMBOY_DestroyProgram(s_Program);
        exit(EXIT_FAILURE);
    }

    // Set the frame and audio mix callbacks.
    TOMBOY_SetCallbacks(s_Engine, TOMBOY_OnFrameRendered, TOMBOY_OnAudioMix);

    // Set up the window's title.
    const char* l_ProgramTitle = TOMBOY_GetProgramTitle(s_Program);
    char l_WindowTitle[256];
    if (l_ProgramTitle != NULL && l_ProgramTitle[0] != '\0')
    {
        snprintf(l_WindowTitle, sizeof(l_WindowTitle), "TOMBOY - %s", l_ProgramTitle);
    }
    else
    {
        snprintf(l_WindowTitle, sizeof(l_WindowTitle), "TOMBOY - Untitled Program");
    }

    // Create the SDL window.
    s_Window = SDL_CreateWindow(
        l_WindowTitle,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800, 720,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );
    if (s_Window == NULL)
    {
        fprintf(stderr, "Failed to create SDL window: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    // Create the SDL renderer.
    s_Renderer = SDL_CreateRenderer(s_Window, -1, SDL_RENDERER_ACCELERATED);
    if (s_Renderer == NULL)
    {
        fprintf(stderr, "Failed to create SDL renderer: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    // Create the SDL texture for rendering.
    s_RenderTarget = SDL_CreateTexture(
        s_Renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        TOMBOY_PPU_SCREEN_WIDTH,
        TOMBOY_PPU_SCREEN_HEIGHT
    );
    if (s_RenderTarget == NULL)
    {
        fprintf(stderr, "Failed to create SDL texture: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    // Prepare the audio device.
    SDL_AudioSpec l_DesiredSpec = { 0 }, l_ObtainedSpec = { 0 };
    l_DesiredSpec.freq = TOMBOY_AUDIO_SAMPLE_RATE;
    l_DesiredSpec.format = AUDIO_F32;
    l_DesiredSpec.channels = 2;
    l_DesiredSpec.samples = TOMBOY_AUDIO_SAMPLE_SIZE / 2;
    l_DesiredSpec.callback = NULL;
    l_DesiredSpec.userdata = NULL;
    s_AudioDevice = SDL_OpenAudioDevice(NULL, 0, &l_DesiredSpec, &l_ObtainedSpec, 0);
    if (s_AudioDevice == 0)
    {
        fprintf(stderr, "Failed to open audio device: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    
    // Allocate the audio buffer.
    s_AudioBuffer = TM_calloc(TOMBOY_AUDIO_SAMPLE_SIZE, float);
    if (s_AudioBuffer == NULL)
    {
        fprintf(stderr, "Failed to allocate audio buffer.\n");
        exit(EXIT_FAILURE);
    }

    // Begin playing audio.
    SDL_PauseAudioDevice(s_AudioDevice, 0);
    
}

static void TOMBOY_AtExit ()
{
    if (s_AudioBuffer != NULL)              { TM_free(s_AudioBuffer); }
    if (s_AudioDevice != 0)                 { SDL_CloseAudioDevice(s_AudioDevice); }
    if (s_RenderTarget != NULL)             { SDL_DestroyTexture(s_RenderTarget); }
    if (s_Renderer != NULL)                 { SDL_DestroyRenderer(s_Renderer); }
    if (s_Window != NULL)                   { SDL_DestroyWindow(s_Window); }
    if (s_Engine != NULL)                   { TOMBOY_DestroyEngine(s_Engine); }
    if (s_Program != NULL)                  { TOMBOY_DestroyProgram(s_Program); }

    SDL_Quit();
}

// Main Function ///////////////////////////////////////////////////////////////////////////////////

int main (int argc, char** argv)
{
    if (argc < 2)
    {
        printf("Usage: %s <ROM file>\n", argv[0]);
        return 0;
    }

    // Set the exit function, then run the start function.
    atexit(TOMBOY_AtExit);
    TOMBOY_AtStart(argv[1]);

    // Tick the engine until the program is finished.
    while (true)
    {
        if (TOMBOY_TickEngine() == false)
        {
            break;
        }
    }

    return 0;
}
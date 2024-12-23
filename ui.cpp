#include "ui.hpp"
// Global Variables
ID3D11Device* g_pd3dDevice = nullptr;
ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
IDXGISwapChain* g_pSwapChain = nullptr;
ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Function Prototypes
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Main code
void init(Test* te, PriceData pd, ExternData ed, CompData cd)
{
    CompData rd;
    double ad=0, pue=0, poe=0, acc_cutoff = 0.01;


    // Create application window
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("Centauri Winh"), NULL };
    RegisterClassEx(&wc);
    HWND hwnd = CreateWindow(wc.lpszClassName, _T("Centauri Trading Algorithm Tester"), WS_OVERLAPPEDWINDOW, 100, 100, 1000, 650, NULL, NULL, wc.hInstance, NULL);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        UnregisterClass(wc.lpszClassName, wc.hInstance);
        return;
    }

    // Show the window
    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoTitleBar;
    window_flags |= ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoResize;
    //ShowWindow(hwnd, SW_HIDE);
    // Main loop
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));

    ImFont* title_body = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\tahoma.ttf", 32.0f, NULL, io.Fonts->GetGlyphRangesDefault());
    ImFont* font_body = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\tahoma.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesDefault());
    bool avd = 0, opg = 0, ph=0, pl=0;
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f)); // place the next window in the top left corner (0,0)
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::Begin("Centauri", 0, window_flags);
        ImGui::PushFont(title_body);
        ImGui::SeparatorText("Trading Algorithm Tester - ProtoType 0.01");
        ImGui::PopFont();
        ImGui::PushFont(font_body);
        if (ImGui::CollapsingHeader("Output Options", ImGuiTreeNodeFlags_DefaultOpen)) {

            ImGui::PushItemWidth(125);
            /* Avg Distance */
            ImGui::Checkbox("Display Avg Difference", &avd);
            ImGui::SameLine(300); ImGui::Text("Display average difference between true and outputted values.");

            /* Graph */
            ImGui::Checkbox("Display Output Graph", &opg);
            ImGui::SameLine(300); ImGui::Text("Display histograms comparing true and output data.");

            /* Perc > */
            ImGui::Checkbox("Display % Overestimate", &ph);
            ImGui::SameLine(300); ImGui::Text("Display %% of data that overshot target value.");

            /* Perc < */
            ImGui::Checkbox("Display % Underestimate", &pl);
            ImGui::SameLine(300); ImGui::Text("Display %% of data that undershot target value.");

            ImGui::InputDouble("Accuracy Cutoff", &acc_cutoff, 0.001f, 0.01f);
            ImGui::SameLine(300); ImGui::Text("When calculating accuracy, within what range should be considered accurate?");
            ImGui::PopItemWidth();
        }

        if (ImGui::CollapsingHeader("Output Data")) {
            static float arr[] = { 0.6f, 0.1f, 1.0f, 0.5f, 0.92f, 0.1f, 0.2f };
            if (avd) {
                ImGui::Text("Average Difference: %f", ad);
            }
            if (opg) {
                if (rd.cd.size()) {
                    auto cdv = get_so_values(cd);
                    auto rdv = get_so_values(rd);
                    size_t maxima = max(
                        *(std::max_element(cdv.begin(), cdv.end())),
                        *(std::max_element(rdv.begin(), rdv.end()))
                    );

                    ImGui::PlotHistogram("True", get_so_values(cd).data(), cd.cd.size(), 0, NULL, 0.0f, maxima, ImVec2(800, 100.0f));
                    ImGui::PlotHistogram("Outputs", get_so_values(rd).data(), rd.cd.size(), 0, NULL, 0.0f, maxima, ImVec2(800, 100.0f));
                }
            }
            if (ph) {
                ImGui::Text("Overestimated: %f%%", 100 * poe);
            }
            if (pl) {
                ImGui::Text("Underestimated: %f%%", 100 * pue);
            }
            ImGui::Text("Accuracy: %f%%", 100 * te->accuracy(rd, cd, acc_cutoff));
        }

        if (ImGui::Button("Calculate!", ImVec2(100, 50))) {
             rd = te->execute(pd, ed, cd);
             ad = te->avg_diff(rd, cd);
             auto xdfff = te->estimates(rd, cd); //good var naame
             pue = xdfff.first;
             poe = xdfff.second;

        }

        ImGui::PopFont();
        ImGui::End();

        // Rendering
        ImGui::Render();
        const float clear_color_with_alpha[4] = { 0.45f, 0.55f, 0.60f, 1.00f };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0); // Present with vsync
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    DestroyWindow(hwnd);
    UnregisterClass(wc.lpszClassName, wc.hInstance);

    return;
}

// Helper functions
bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2,
        D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
}

// Win32 message handler
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_DROPFILES:
        break;
    default:
        break;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

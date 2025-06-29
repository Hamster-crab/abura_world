#include <iostream>
#include <raylib.h>
#include <raymath.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include "fonts/8bitBold.h"

struct PlayerData {
    int x;
    int y;
    int room;
    int hp;
    int thirst;
    int attackone;
    int attacktwo;
    int attackthree;
};

Rectangle windowSize = {0, 0, 800, 500};
std::filesystem::path saveDir;
std::filesystem::path savePath;
PlayerData inGamePlayerData;

static float cameraVelocityY = 0.0f;    // カメラのY方向の速度
static float jumpStrength = 1.0f;     // ジャンプの強さ (上向き)
static float gravity = 3.2f;           // 重力加速度 (下向き)
static float terminalVelocity = 6.0f; // 落下速度の最大値
static bool canJump = true;            // ジャンプできるかどうか (地面にいるか)
static float groundY = 1.0f;

std::filesystem::path GetSaveGameDirectory() {
    std::filesystem::path savePath;
    #ifdef _WIN32
    // Windows: C:\Users\<UserName>\Saved Games または Documents
    // 実際にはSHGetKnownFolderPathなどを使うのが確実
    const char* homeDir = std::getenv("USERPROFILE");
    if (homeDir) {
        savePath = std::filesystem::path(homeDir) / "Saved Games" / "abura_world";
    } else {
        savePath = std::filesystem::current_path() / "saves"; // フォールバック
    }
    #elif __APPLE__
    // macOS: ~/Library/Application Support/YourGameName
    // 実際には NSSearchPathForDirectoriesInDomains などを使うのが確実
    const char* homeDir = std::getenv("HOME");
    if (homeDir) {
        savePath = std::filesystem::path(homeDir) / "Library" / "Application Support" / "abura_world";
    } else {
        savePath = std::filesystem::current_path() / "saves"; // フォールバック
    }
    #else // Linux/Unix
    // Linux: ~/.local/share/YourGameName
    const char* xdgDataHome = std::getenv("XDG_DATA_HOME");
    if (xdgDataHome) {
        savePath = std::filesystem::path(xdgDataHome) / "abura_world";
    } else {
        const char* homeDir = std::getenv("HOME");
        if (homeDir) {
            savePath = std::filesystem::path(homeDir) / ".local" / "share" / "abura_world";
        } else {
            savePath = std::filesystem::current_path() / "saves"; // フォールバック
        }
    }
    #endif
    return savePath;
}

PlayerData loadGame(const std::filesystem::path& filepath) { // <-- ここを変更
    // デフォルト値で初期化 (attackthreeも忘れずに)
    PlayerData data{0, 0, 0, 100, 0, 0, 0, 0};

    // std::ifstream のコンストラクタに std::filesystem::path を直接渡す
    std::ifstream file(filepath); // <-- ここを変更
    std::string line;

    // ファイルが開けなかった場合のチェックは非常に重要
    if (!file.is_open()) {
        std::cerr << "Failed to open file for loading: " << filepath << std::endl; // エラーメッセージもpath型で
        return data; // 開けなかった場合はデフォルトデータを返す
    }

    while (std::getline(file, line)) {
        // 行頭/行末の空白やCR/LFを除去 (オプションだが推奨)
        // macOS/LinuxでのCR(\r)やWindowsでのCR+LF(\r\n)に対応するため
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);


        std::istringstream iss(line);
        std::string key;
        if (std::getline(iss, key, '=')) {
            std::string valueStr;
            if (std::getline(iss, valueStr)) {
                // key と valueStr からも余分な空白を除去するとさらに堅牢
                key.erase(0, key.find_first_not_of(" \t"));
                key.erase(key.find_last_not_of(" \t") + 1);
                valueStr.erase(0, valueStr.find_first_not_of(" \t"));
                valueStr.erase(valueStr.find_last_not_of(" \t") + 1);

                try {
                    int value = std::stoi(valueStr);
                    if (key == "x") data.x = value;
                    else if (key == "y") data.y = value;
                    else if (key == "room") data.room = value;
                    else if (key == "hp") data.hp = value;
                    else if (key == "thirst") data.thirst = value;
                    else if (key == "attackone") data.attackone = value;
                    else if (key == "attacktwo") data.attacktwo = value;
                    else if (key == "attackthree") data.attackthree = value;
                    // else {
                    //    std::cerr << "Unknown key: " << key << std::endl; // 未知のキーを検出するのも良い
                    // }
                } catch (const std::invalid_argument& e) {
                    std::cerr << "Invalid number format for value: '" << valueStr << "' in line: '" << line << "'" << std::endl;
                } catch (const std::out_of_range& e) {
                    std::cerr << "Value out of range: '" << valueStr << "' in line: '" << line << "'" << std::endl;
                }
            } else {
                std::cerr << "Missing value for key: '" << key << "' in line: '" << line << "'" << std::endl;
            }
        } else {
            // `=` が見つからない行は無視または警告
            // std::cerr << "Skipping malformed line (no '='): '" << line << "'" << std::endl;
        }
    }

    file.close(); // ファイルを閉じる
    return data;
}

void drawTitle(Font myFont, bool& title) {
    ShowCursor();
    saveDir = GetSaveGameDirectory();
    savePath = saveDir / "save.txt";

    int mouseX = GetMouseX();
    int mouseY = GetMouseY();

    const char* titleText = "abura_world";
    const char* startText = "start";
    const char* resetText = "reset";
    const char* exitText = "exit";
    const int spacing = 2;

    Vector2 titleTextSize = MeasureTextEx(myFont, titleText, 50, (float)spacing);
    Vector2 startTextSize = MeasureTextEx(myFont, startText, 24, (float)spacing);
    Vector2 resetTextSize = MeasureTextEx(myFont, resetText, 24, (float)spacing);
    Vector2 exitTextSize = MeasureTextEx(myFont, exitText, 24, (float)spacing);

    float titlePosX = (GetScreenWidth() - titleTextSize.x) / 2.0f;
    float titlePosY = (GetScreenHeight() - titleTextSize.y) / 17.0f;

    float startPosX = (GetScreenWidth() - startTextSize.x) / 2.0f;
    float startPosY = (GetScreenHeight() - startTextSize.y) / 1.5f;

    float resetPosX = (GetScreenWidth() - resetTextSize.x) / 2.0f;
    float resetPosY = startPosY + 50;

    float exitPosX = (GetScreenWidth() - exitTextSize.x) / 2.0f;
    float exitPosY = resetPosY + 50;

    BeginDrawing();
    ClearBackground(RAYWHITE);

    if (mouseX >= startPosX && mouseX <= startPosX+24*5 && mouseY >= startPosY && mouseY <= startPosY+24) {
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            inGamePlayerData = loadGame(savePath);
            title = false;
        }
        DrawRectangleLines(startPosX, startPosY, 24*5, 24, {255, 0, 0, 255});
    } else if (mouseX >= resetPosX && mouseX <= resetPosX+24*5 && mouseY >= resetPosY && mouseY <= resetPosY+24) {
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            std::string sampleWriteData = R"(x=30
y=750
room=1
hp=80
thirst = 50
attackone=1
attacktwo=2
attackthree=3)";
            try {
                std::filesystem::create_directories(saveDir);

                std::ofstream ofs(savePath, std::ios_base::out | std::ios_base::trunc);

                if (ofs.is_open()) {
                    ofs << sampleWriteData; // データをファイルに書き込む
                    ofs.close(); // ファイルを閉じる
                    std::cout << "Data saved to '" << savePath << "'." << std::endl;
                } else {
                    std::cerr << "Failed to open/create file '" << savePath << "' for writing." << std::endl;
                }
            } catch (const std::filesystem::filesystem_error& e) {
                std::cerr << "Filesystem error during save: " << e.what() << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "General error during save: " << e.what() << std::endl;
            }
        }
        DrawRectangleLines(resetPosX, resetPosY, 24*5, 24, {255, 0, 0, 255});
    } else if (mouseX >= exitPosX && mouseX <= exitPosX+24*5 && mouseY >= exitPosY && mouseY <= exitPosY+24) {
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            CloseWindow();
            std::cout << "exit" << std::endl;
        }
        DrawRectangleLines(exitPosX, exitPosY, 24*4, 24, {255, 0, 0, 255});
    }

    DrawTextEx(myFont, titleText, (Vector2){ titlePosX, titlePosY }, 50, (float)spacing, BLACK);
    DrawTextEx(myFont, startText, (Vector2){ startPosX, startPosY }, 24, (float)spacing, BLACK);
    DrawTextEx(myFont, resetText, (Vector2){ resetPosX, resetPosY }, 24, (float)spacing, BLACK);
    DrawTextEx(myFont, exitText, (Vector2){ exitPosX, exitPosY }, 24, (float)spacing, BLACK);
    EndDrawing();
}

void mainGame(Vector3 cubePosition, Camera3D& camera) {
    HideCursor();
    float deltaTime = GetFrameTime();

    // --- 重力とジャンプの物理演算 ---

    // 1. 重力の適用
    // 下方向に速度を加算 (Y軸は通常上方向が正、下方向が負なので、重力は正の値を加える)
    cameraVelocityY -= gravity * deltaTime; // 重力でY速度を減少させる (下方向への加速)

    // 2. 落下速度の制限 (ターミナルベロシティ)
    // 落下が速くなりすぎないように制限
    if (cameraVelocityY < -terminalVelocity) {
        cameraVelocityY = -terminalVelocity;
    }

    // 3. ジャンプ入力
    // スペースキーが押され、かつジャンプ可能な場合
    if (IsKeyPressed(KEY_SPACE) && canJump) {
        cameraVelocityY = jumpStrength; // 上向きに速度を設定
        canJump = false; // ジャンプ中は再ジャンプ不可
    }

    // 4. カメラ位置の更新
    camera.position.y += cameraVelocityY; // 速度を直接加算する (deltaTimeは速度計算に含める)

    // 5. 地面との衝突判定 (または最低高度の維持)
    if (camera.position.y <= groundY) {
        camera.position.y = groundY;   // 地面に固定
        cameraVelocityY = 0.0f;        // Y方向の速度をリセット
        canJump = true;                // 地面にいるのでジャンプ可能
    }

    // デバッグ出力
    // std::cout << "Camera Y: " << camera.position.y << ", Velocity Y: " << cameraVelocityY << ", Can Jump: " << canJump << std::endl;

    // --- Raylib カメラの更新と描画 ---
    UpdateCamera(&camera, CAMERA_FIRST_PERSON); // カメラの視点移動など (キー入力による)

    BeginDrawing();
    ClearBackground(RAYWHITE);
    BeginMode3D(camera);

    DrawCube(cubePosition, 2.0f, 2.0f, 2.0f, RED);
    DrawCubeWires(cubePosition, 2.0f, 2.0f, 2.0f, MAROON);

    DrawGrid(10, 1.0f); // 地面のようなグリッドを描画

    EndMode3D();
    DrawFPS(10, 10); // フレームレート表示
    EndDrawing();
}

int main(void)
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(windowSize.width, windowSize.height, "abura");

    SetExitKey(KEY_NULL);

    Camera3D camera = { 0 };
    camera.position = (Vector3){ 10.0f, 10.0f, 10.0f }; // Camera position
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type

    Vector3 cubePosition = { 0.0f, 0.0f, 0.0f };

    Font myFont = LoadFontFromMemory(
        ".ttf", // フォントのフォーマット
        __8bitBold_ttf, // 8bitBold.h で定義されたバイト配列名 (例: eightBitBoldFont)
    __8bitBold_ttf_len, // 8bitBold.h で定義されたサイズ変数名 (例: eightBitBoldFont_SIZE)
    1000, // フォントサイズ
    nullptr,
    0);

    bool title = true;

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        windowSize.width = GetScreenWidth();
        windowSize.height = GetScreenHeight();

        if (title) {
            drawTitle(myFont, title);
        } else {
            mainGame(cubePosition, camera);
        }
    }

    UnloadFont(myFont);
    CloseWindow();

    return 0;
}

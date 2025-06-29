cmake_minimum_required(VERSION 3.10) # CMakeの最小バージョンを指定

project(main CXX) # プロジェクト名と使用する言語を指定

# raylibのインストールパスを設定（必要に応じて変更してください）
# もしraylibがシステムにインストールされている場合は不要な場合があります。
# set(RAYLIB_PATH "C:/raylib/raylib") # Windowsの場合の例
# set(RAYLIB_PATH "/usr/local") # Linux/macOSの場合の例
# あるいは、find_package()を使用する場合は以下のようにします。
# find_package(raylib CONFIG REQUIRED)

# raylibのソースファイルまたはライブラリへのパスを追加
# raylibをソースからビルドする場合
# add_subdirectory(${RAYLIB_PATH} ${CMAKE_BINARY_DIR}/raylib_build)
# target_include_directories(${PROJECT_NAME} PRIVATE ${RAYLIB_PATH}/src)
# target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

find_package(raylib CONFIG REQUIRED) # raylibの設定ファイルを探します

# 実行可能ファイルを生成
add_executable(${PROJECT_NAME}
    src/main.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include # <-- ここで 'include' ディレクトリを追加
)

# raylibライブラリにリンク
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

# 必要なインクルードディレクトリを追加（raylibのヘッダーファイルが見つかるように）
# find_package()を使用している場合、通常は自動的に設定されますが、明示的に追加することもできます。
# target_include_directories(${PROJECT_NAME} PRIVATE ${RAYLIB_INCLUDE_DIR}) # find_packageが設定する変数

# macOSの場合、フレームワークのリンクが必要になることがあります
if(APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework Cocoa" "-framework IOKit" "-framework CoreVideo" "-framework OpenGL")
endif()

# Linuxの場合、追加のライブラリが必要になることがあります
if(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE m pthread dl rt)
endif()

# ビルドディレクトリの設定
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

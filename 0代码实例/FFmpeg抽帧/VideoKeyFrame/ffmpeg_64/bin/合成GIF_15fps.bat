@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion
color 0A
title 【100%匹配C++源码】GIF合成工具

:: ====================== 你只改这些 ======================
set "INPUT_VIDEO=input.mp4"         :: 素材视频
set "TEMPLATE_FOLDER=15fps"         :: 模板文件夹
set "maFrequencyExtrack=1"          :: 抽帧间隔
set "FPS=15"                        :: GIF帧率
:: ========================================================

set "maStartFrame=0"
set "maEndFrame=9999"
set "tplFrames=75"
set "OUT_FOLDER=testFPS"

if not exist "%OUT_FOLDER%" mkdir "%OUT_FOLDER%"
for %%f in (%INPUT_VIDEO%) do set "SRC_NAME=%%~nf"
set "GIF_NAME=!SRC_NAME!-!TEMPLATE_FOLDER!-!maFrequencyExtrack!.gif"

cls
echo.
echo ====================== START PROCESS ======================
echo Video: !INPUT_VIDEO!
echo Interval: !maFrequencyExtrack!
echo FPS: !FPS!
echo Output: !GIF_NAME!
echo.

:: ====================== 1. 抽帧 ======================
echo [1/3] 抽帧中...
ffmpeg -loglevel error -hide_banner -i "!INPUT_VIDEO!" ^
-filter_complex "[0:v]crop=x='if(lte(iw,ih),0,0.5*iw-0.5*ih)':y='if(lte(ih,iw),0,0.5*ih-0.5*iw)':w='if(lte(iw,ih),iw,ih)':h='if(lte(ih,iw),ih,iw)'[c0];[c0]scale=240x240[cs0];[cs0]select=expr='between(n,!maStartFrame!,!maEndFrame!)*not(mod(n-!maStartFrame!,!maFrequencyExtrack!))',setpts=N/FRAME_RATE/TB" ^
-y "%OUT_FOLDER%\mat_%%05d.png"

:: 统计抽了多少帧
set /a maSelectedFramesCnt=0
for %%f in (%OUT_FOLDER%\mat_*.png) do set /a maSelectedFramesCnt+=1
echo 实际抽帧数: !maSelectedFramesCnt!

:: ====================== 2. 复制逻辑 ======================
echo [2/3] 复制补齐到50帧...
if !maSelectedFramesCnt! equ 0 (
    echo ERROR: 未抽到任何帧
    pause
    exit /b
)

set /a out=0
if !maSelectedFramesCnt! geq !tplFrames! (
    :: 情况 A：抽帧 >=50 → 取前50张
    for /l %%i in (1,1,50) do (
        set "s=0000%%i"
        set "s=!s:~-5!"
        set "d=00000!out!"
        set "d=!d:~-6!"
        copy /y "%OUT_FOLDER%\mat_!s!.png" "%OUT_FOLDER%\copy_!d!.png" >nul
        set /a out+=1
    )
) else (
    :: 情况 B：抽帧 <50 → C++ 均匀复制算法
    set /a maSelectedFrameRepeatCnt=(tplFrames + maSelectedFramesCnt - 1^) / maSelectedFramesCnt
    set /a leftFrameCnt=tplFrames
    set /a iSelectedFrame=0
    set /a part1=tplFrames - (maSelectedFrameRepeatCnt - 1^) * maSelectedFramesCnt

    :copy_loop
    if !leftFrameCnt! gtr 0 if !iSelectedFrame! lss !maSelectedFramesCnt! (
        if !iSelectedFrame! lss !part1! (
            set "tmpRepeatCnt=!maSelectedFrameRepeatCnt!"
        ) else (
            set /a tmpRepeatCnt=maSelectedFrameRepeatCnt - 1
        )

        set "s=0000!iSelectedFrame!"
        set "s=!s:~-5!"
        for /l %%c in (1,1,!tmpRepeatCnt!) do (
            if !leftFrameCnt! gtr 0 (
                set "d=00000!out!"
                set "d=!d:~-6!"
                copy /y "%OUT_FOLDER%\mat_!s!.png" "%OUT_FOLDER%\copy_!d!.png" >nul
                set /a out+=1
                set /a leftFrameCnt-=1
            )
        )
        set /a iSelectedFrame+=1
        goto copy_loop
    )
)

:: ====================== 3. 合成GIF ======================
echo [3/3] 生成GIF...
ffmpeg -loglevel error -hide_banner ^
-r !FPS! -i "%OUT_FOLDER%\copy_%%06d.png" ^
-r !FPS! -i "!TEMPLATE_FOLDER!/1_%%05d.png" ^
-filter_complex "[0:v][1:v]overlay=0:0" -r !FPS! -pix_fmt rgb24 -y "!GIF_NAME!"

echo.
echo ====================== 完成 ======================
echo GIF: !GIF_NAME!
echo 图片保存在: testFPS\
echo ====================================================
echo.
pause
exit /b
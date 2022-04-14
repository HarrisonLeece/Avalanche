ffmpeg -i output_pattern.webm -i ./wavs/input.wav -map 0:v -map 1:a -c:v copy -shortest av-output.webm

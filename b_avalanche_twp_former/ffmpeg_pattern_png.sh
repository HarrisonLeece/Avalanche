ffmpeg  -framerate 60 -start_number 0 -i ./beat_detection_graph/%d.png -c:v vp9 -threads 10 -crf 15 -b:v 2M -vf "format=rgba" ../output_pattern.webm

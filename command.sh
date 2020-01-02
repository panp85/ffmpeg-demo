#./ffmpeg -i rtmp://58.200.131.2:1935/livetv/hunantv hunan.wav  #2wav
#./ffmpeg -i rtmp://58.200.131.2:1935/livetv/hunantv -acodec copy -vcodec copy -y hunan.flv
./ffmpeg -i rtmp://58.200.131.2:1935/livetv/hunantv -strict -2 -vcodec copy hunan.mp4

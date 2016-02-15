#streamer  -o /dev/shm/QStreamer/0_images/img000000.ppm -f ppm -c /dev/video0 -s 320x240 -t 02:00:00 -r 10
cd photos;
rm *;
mencoder tv:// fps=1:driver=v4l:width=160:height=120:device=/dev/video0 -nosound -ovc lavc -o wc.avi &
sleep 2;
mplayer wc.avi -vf screenshot;
rm wc.avi;
killall mencoder
OK 500k
./slcan_terminal -i C,MF,MD,ME,MS,S6,V,ON /dev/ttyACM0
./slcan_terminal -i C,MF,MD,ME,MS,S6,V,ON /dev/ttyACM1
t11#2345

OK 1M100k
./slcan_terminal -i C,MF,MD,ME,MS,S3,Y1,V,ON /dev/ttyACM0
./slcan_terminal -i C,MF,MD,ME,MS,S3,Y1,V,ON /dev/ttyACM1
b11#2345

OK 5M1M
./slcan_terminal -i C,MF,MD,ME,MS,S8,Y5,V,ON /dev/ttyACM0
./slcan_terminal -i C,MF,MD,ME,MS,S8,Y5,V,ON /dev/ttyACM1
b11#2345

8M1M
./slcan_terminal -i C,MF,MD,ME,MS,S8,Y8,V,ON /dev/ttyACM0
./slcan_terminal -i C,MF,MD,ME,MS,S8,Y8,V,ON /dev/ttyACM1
b11#2345

[INIT] Y0
[RESP] #2 (Invalid parameter)



"s4,69,10,7\r"	
Set nominal bitrate: 
Prescaler=4, Seg1=69, Seg2=10, Synchr. Jump Width=7

"y4,9,10,7\r"	
Set data bitrate: 
Prescaler=4, Seg1=9, Seg2=10, Synchr. Jump Width=7

5M1M
"s1,119,40,40\r"	
"y1,16,15,15\r"	
./slcan_terminal -i C,MF,MD,ME,MS,s\"1,119,40,40\",y\"1,16,15,15\",V,ON /dev/ttyACM0
./slcan_terminal -i C,MF,MD,ME,MS,s\"1,119,40,40\",y\"1,16,15,15\",V,ON /dev/ttyACM1
b11#2345


2M500K
./slcan_terminal -i C,MF,MD,ME,MS,S6,Y2,V,ON /dev/ttyACM0
./slcan_terminal -i C,MF,MD,ME,MS,s\"1,239,80,80\",y\"4,10,9,9\",V,ON /dev/ttyACM0

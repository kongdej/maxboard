# maxboard
Installation

1. Local Server
      - firebase.js  : connect firebase and create /board/<boardId>.html
      - server.js    : local web server ./board/<boardId>.html for Led maxtrix

2. LINEBOT
      - ./linebot/index.js : webhook for line bot, put message to firebase realtime database

3. ESP32 CODE
      #define MAX_DEVICES 16
      #define CLK_PIN   14    
      #define DATA_PIN  12    
      #define CS_PIN    27  
      resetIn = 2

SERVERS
 - Firebase : doitung@xxx / create realtime database
 - LineDev  : kongdej@xxxx / create provider and bot
 - Heroku   : kongdej@xxxx / create app (maxboard.herokuapp.com) then push ./linebot
 - Localhost: 
              - Firebase listen event (child_added, child_updated)
              - webserver (ihear.xxx.xx.x..:9999/<boardId>.html


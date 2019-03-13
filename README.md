# maxboard
kongdejs@gmail.com
13 Mar 2019

User Manual

1. LocalServer (Raspbery Pi): Listen firebase-database, then create ./boards/boardId.html with LINE message and response requested from LEDMATRIX (http://localhost:9999/boardId.html)

      - node index.js 

2. LINEBOT (Heroku): LINEBOT Webhook, put message to firebase-database
      
      - create ./linebot/serviceAccountKey.json 
      - copy to ./linebot
      - deploy to heroku 
      - update webhook url 

3. ESP32 CODE
      #define MAX_DEVICES 16
      #define CLK_PIN   14    
      #define DATA_PIN  12    
      #define CS_PIN    27  
      resetIn = 2

MY NOTES:
 - Firebase : doitung@xxx / create realtime database
 - LineDev  : kongdej@xxxx / create provider and bot
 - Heroku   : kongdej@xxxx / create app (maxboard.herokuapp.com) then push ./linebot
 - Localhost: 
              - Firebase listen event (child_added, child_updated)
              - webserver (ihear.xxx.xx.x..:9999/<boardId>.html


# maxboard
kongdejs@gmail.com
13 Mar 2019

User Manual

1. LINEBOT (Heroku): LINEBOT Webhook, put message to firebase-database
      
      - create ./linebot/serviceAccountKey.json 
      - copy to ./linebot
      - deploy to heroku 
      - update LINE Webhook URL 

2. LocalServer: Listen firebase-database, then create ./boards/boardId.html with LINE message and response requested from LEDMATRIX (http://localhost:9999/boardId.html)

      - node index.js 
      
3. ESP32 CODE

      - modify localhost url


MY NOTES:
 - Firebase : doitung@xxx / create realtime database
 - LineDev  : kongdej@xxxx / create provider and bot
 - Heroku   : kongdej@xxxx / create app (maxboard.herokuapp.com) then push ./linebot
 - Localhost: 
              - Firebase listen event (child_added, child_updated)
              - webserver (ihear.xxx.xx.x..:9999/<boardId>.html


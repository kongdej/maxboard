const express = require('express');
const line = require('@line/bot-sdk');
var admin = require("firebase-admin");

var serviceAccount = require("./serviceAccountKey.json");

admin.initializeApp({
  credential: admin.credential.cert(serviceAccount),
  databaseURL: "https://maxboard-egat.firebaseio.com"
});

const ref = admin.app().database()
const usersRef = ref.ref('/users')
const boardRef = ref.ref('/board')

const app = express();

const config = {
    channelAccessToken: "84ah/gtpG/bWLWtDU3COD+6YDnfEy7of4Zy/7QuFCrY/fNyEfYPprP4lsOMK0hXULqsXcqXztXcMDsfFEwrx5jX3KelVvz1iwFvj+CXzMtiGlVUmbYaP+PuAJyva2KpDdM9ZoNXgom7x0PyMB6p8YQdB04t89/1O/w1cDnyilFU=",
    channelSecret: "c15c64dda7f40766228fc05fa2ca02b9"
};

const client = new line.Client(config);

app.post('/webhook', line.middleware(config), (req, res) => {
    Promise
        .all(req.body.events.map(handleEvent))
        .then((result) => res.json(result));
});

function handleEvent(event) {
    if (event.type === 'message' && event.message.type === 'text') {

        usersRef.once('value', function (snapshot) {
            console.log(snapshot.key)
            let found = 0
            snapshot.forEach(function (childSnapshot) {
                if (childSnapshot.key == event.source.userId) {
                    console.log("@" + childSnapshot.val().boardname + ':' + event.message.text)
                    found = 1
                    if (event.message.text.substring(0, 1) == "@") {
                        let userRef = usersRef.child(event.source.userId);
                        userRef.set({ 'boardname': event.message.text.substring(1) });
                        handleMessageEvent(event, "ติดต่อ '" + event.message.text.substring(1) + "' สำเร็จ");
                    }
                    else {
                        boardRef.child(childSnapshot.val().boardname).set({
                            'message' : event.message.text
                        });
                        handleMessageEvent(event, "@" + childSnapshot.val().boardname + ':' + event.message.text);
                    }
                }
            })
            if (!found) {
                console.log('กรุณาป้อน @<boardname>')
                if (event.message.text.substring(0, 1) == "@") {
                    let userRef = usersRef.child(event.source.userId);
                    userRef.set({ 'boardname': event.message.text.substring(1) });
                    handleMessageEvent(event, "ติดต่อ '" + event.message.text.substring(1) + "' สำเร็จ");
                }
                else {
                    handleMessageEvent(event, "กรุณาป้อน @<boardname>");
                }
            }
        });
    } else {
        return Promise.resolve(null);
    }

}

function handleMessageEvent(event, text) {
    var msg = {
        type: 'text',
        text: text
    };

    return client.replyMessage(event.replyToken, msg);
}

app.set('port', (process.env.PORT || 80));

app.listen(app.get('port'), function () {
    console.log('run at port', app.get('port'));
});
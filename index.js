const firebase = require("firebase");
const fs = require('fs')

firebase.initializeApp({
    "serviceAccount": "./maxboard.json",
    "databaseURL": "https://maxboard-egat.firebaseio.com"
});

const ref = firebase.app().database().ref('/board');

ref.on('child_changed', function (data) {
    console.log(data.key, '=', data.val())
    let path = './boards/' + data.key + '.html'
    let msg = data.val().message
    fs.writeFile(path, msg, { flag: 'w' }, function (err) {
        if (err) throw err;
    })
});

ref.on('child_added', function (data) {
    let path = './boards/' + data.key + '.html'
    fs.exists(path, function (exists) {
        if (!exists) {
            let msg = data.val().message
            fs.writeFile(path, msg, { flag: 'wx' }, function (err) {
                if (err) throw err;
                console.log("It's saved!");
            });
        }
        else {
            console.log("Refusing to overwrite existing", path);
        }
    });


});

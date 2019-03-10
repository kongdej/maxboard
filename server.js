const express = require('express');
const app = express();

const port = 9999;
app.use(express.static('boards'));
app.listen(port, function() {
    console.log('Starting server on port ' + port);
});
var express = require('express');
var ipc = require('node-ipc');

var app = express();

var server = app.listen(3333, () => {
  

})

var bodyParser = require('body-parser')
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({extended: false}))

var io = require('socket.io')(server);

console.log("running on port", server.address().port);
app.use(express.static(__dirname));



io.on('connection', () =>{
  console.log('a user is connected')
})

app.get('/msg', (req, res) => {
  var user = req.params.user
})

/////////////////// IPC /////////////////////////


ipc.config.id = 'node'
ipc.config.retry = 1000

ipc.connectTo('bot', )


//////////////////////////////////////////////////


app.post('/msg', (req, res) => {
  console.log(req.body);
  handle = req.body.name;
  msg = req.body.message;
  io.emit('public_message', req.body);
  console.log('received message from', handle, 'content:', msg);
  // res.redirect('back')
});
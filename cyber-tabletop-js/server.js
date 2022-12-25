var express = require('express');                           //
var net = require('net');

var app = express();                                        //

var server = app.listen(3333, () => {                       //
})                                                          //

var bodyParser = require('body-parser')                     //
app.use(bodyParser.json());                                 //
app.use(bodyParser.urlencoded({extended: false}))           //

var io = require('socket.io')(server);                

console.log("running on port", server.address().port);
app.use(express.static(__dirname)); 

users = new Map();    

app.get('/msg', (req, res) => {   
  var user = req.params.user    
})      

/////////////////// IPC /////////////////////////



const bot  = net.Socket();    
bot.connect('/tmp/conn');   
bot.on('error', (e) => {    
  console.error('socket error occured!');   
  console.error(e);
});
bot.on('close', (hasErr) => {
  // if (hasErr) {
    console.log('connection to chat bot lost, reconnecting in 2 seconds...')
    setTimeout(() => {bot.connect('/tmp/conn')}, 2000);
  // }
})

bot.on('connect', () => {
  console.log('socket connection to chat bot established.');
})

//////////////////////////////////////////////////
// console.log('reached here');
bot.on('data', (mstr) => {
  m = JSON.parse(mstr);
  if (m.public) {
    io.emit('from_server', m);
  } else {
    users[m.user].emit('from_server', m);
  }
});

io.on('connection', (socket) =>{
  let u = socket.handshake.auth.login;
  users[u] = socket;
  console.log('user', u, 'connected.');
  

socket.on('from_client', (m) => {
  bot.write(JSON.stringify(m));
  console.log(m.user, 'sent a', m.public ? "public" : "private", 'message:', m.content);
  if (m.public) {
    io.emit('from_server', m);
  }
});
})

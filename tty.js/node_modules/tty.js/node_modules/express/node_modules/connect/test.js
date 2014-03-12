
var connect = require('./');
var app = connect();

app.use(connect.directory('.'));

app.listen(3000)
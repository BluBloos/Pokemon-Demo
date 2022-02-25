const express = require('express');
const app = express();
const port = 3000;

app.use(express.static(`${__dirname}/`)); // specify the root directory from which to serve static assets.
//app.get(/.*/, (req, res) => res.sendFile(`frontend/build/index.html`, { root: __dirname }));

app.listen(port, () => {
  console.log(`Server running on port:${port}`)
})
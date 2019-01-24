const express = require('express');
const app = express();
const bodyParser = require('body-parser');
const shell = require('shelljs');
const fs = require('fs');

app.set('view engine', 'ejs');
app.use(express.static('public'));
app.use(bodyParser.urlencoded({ extended: true }));

app.get('/', function (req, res) {
    res.render('index', {results: null});
});

app.post('/', function (req, res) {
    let phrase = req.body.phrase;

    if (shell.exec('../foogle_phrase.sh ' + phrase).code === 0) {
        // Wczytaj jsona z wynikami
        let obj = JSON.parse(fs.readFileSync('./results.json', 'utf8'));

        // Posortuj wyniki po hitach
        let results = obj.results.sort(function(a, b) {return b.hits - a.hits;});

        res.render('index', {results: results, phrase: phrase});
    } else {
        res.render('index', {results: null});
    }
});

app.listen(3000, function () {
    console.log('foogle listening on port 3000!')
});
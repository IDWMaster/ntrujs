var os = require('os');
var child_process = require('child_process');
var fs = require('fs');

fs.copy = function (src, dest, callBach) {
    fs.open(src, 'r', function (err, rfd) {
        fs.open(dest, 'w+', function (err, wfd) {
            var srcStream = fs.createReadStream(null, { fd: rfd });
            var destStream = fs.createWriteStream(null, { fd: wfd });
            srcStream.pipe(destStream);
            srcStream.on('close', function () {
                callBach();
            });
        });
    });
};

var run_installer = function () {
    child_process.exec('(node-gyp rebuild) || (exit 0)', function (err, stdout, stderr) {
        console.log(stdout);
        console.log(stderr);
        process.exit(0);
    });
};

if (os.platform().indexOf('win') > -1) {
    //Windows
    fs.copy('binding_windows.gyp', 'binding.gyp', function () {
        run_installer();
    });
} else {
    //Assume Linux for now
    fs.copy('binding_linux.gyp', 'binding.gyp', function () {
        run_installer();
    });
}
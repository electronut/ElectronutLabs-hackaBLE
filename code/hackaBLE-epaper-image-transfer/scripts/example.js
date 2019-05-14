const _cliProgress = require('cli-progress');
 
// create a new progress bar instance and use shades_classic theme
const bar1 = new _cliProgress.Bar({}, _cliProgress.Presets.shades_classic);
 
// start the progress bar with a total value of 200 and start value of 0
bar1.start(200, 0);
 
// update the current value in your application..
bar1.update(150);
 
// stop the progress bar
bar1.stop();
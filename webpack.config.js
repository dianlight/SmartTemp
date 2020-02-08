const path = require('path');
const CompressionPlugin = require('compression-webpack-plugin');
const TerserPlugin = require('terser-webpack-plugin');
const HtmlReplaceWebpackPlugin = require('html-replace-webpack-plugin');
const HtmlWebpackPlugin = require('html-webpack-plugin');

let read = 0;
let genv,gargv;


var config = {
  entry: {
    main: path.resolve(__dirname, './src/node/index.js'),
    //    remoteDebugApp: path.resolve(__dirname, './src/node/RemoteDebugApp/index.html')
  },
  output: {
    //    filename: '[name].[id].[ext]',
    path: path.resolve(__dirname, 'data'),
    //    publicPath: "/"
  },
  plugins: [
    new HtmlWebpackPlugin({
      template: '!!html-loader!src/node/index.html',
    }),
    new HtmlReplaceWebpackPlugin([
      {
        pattern: '@@title',
        replacement: 'html replace webpack plugin'
      },
      {
        pattern: /(\{\{ ([\w-_\.\(\)\/]+) \s*\}\})/g,
        replacement: function(match, command) {
          // those formal parameters could be:
          // match: <-- env:SMT_VERSION-->
          // type: argv, argvenv, webpackenv, npmenv
          // value: SMT_VERSION
//          console.log("--------",genv,"----------",process.env,"----------",gargv,'-------------');
//          console.log(match,command);
          let argv = gargv;
          const titleize = require('titleize');
          console.log(command,JSON.stringify(eval(command)));
          return eval(command);
        }
      }
    ])
  ],
  module: {
    rules: [
      { test: /\.css(\?mtime=\d+)?$/i, use: ['style-loader', 'css-loader'] },
      {
        test: /\.jpe?g$|\.ico$|\.gif$|\.png$|\.svg$|\.woff$|\.ttf$|\.wav$|\.mp3$/,
        loader: 'file-loader?name=[name].[ext]'  // <- retain original file name
      },
      {
        test: /\.html$/,
        use: ['file-loader?name=[name].[ext]',
          {
            loader: 'extract-loader',
            options: {
              name: '[path][name].[ext]',
              //                  publicPath: (url,resourcePath,context) => { 
              //                    console.log("----------------------------------------------------------");
              //                    console.log(url,resourcePath,context);
              //                    return '../'.repeat(path.relative(path.resolve('src/node'), context.context).split('/').length); 
              //                  },
            }
          },
          {
            loader: 'html-loader',
            options: {
              minimize: true,
              interpolate: true,
              esModule: false,
              attrs: ['img:src', 'link:href', 'iframe:src'],
            }
          },
        ]
      },
      {
        test: /\.js$/,
        use: [
          "source-map-loader",
        ],
      }
    ]
  },
  optimization: {
    minimize: true,
    minimizer: [new TerserPlugin()],
  },
  devServer: {
    contentBase: path.join(__dirname, 'data'),
    compress: true,
    proxy: {
      '/ws':{
        target: 'ws://localhost:9001',
        ws: true
      }
    },
    port: 9000,
    before: function (app, server, compiler) {
      //      var bodyParser = require('body-parser');
      //      app.use(bodyParser.json());
      // console.log(server);
      var WebSocketServer = require("ws").Server;
      var wss = new WebSocketServer({port: 9001});
      wss.on("connection", function(ws){
      //  console.log("Connesso WS",ws,ws._socket.server);
        ws.on('message', function(msg) {
          console.log("--> %s",msg);
          ws.send(msg);
        });
        ws.send('hello!');
      });

      app.get('/load', function (req, res) {
        res.json({ 
          "teco": 18.5, 
          "tnorm": 19.5, 
          "tconf": 20.5, 
          "aautotimeout": 30, 
          "dsleep": 35, 
          "doff": 60, 
          "tadelta": -0.5, 
          "tamode": 3, 
          "tprec": 0.2, 
          "sres": 30, 
          "nname": "SmartTemp", 
          "saddress": "hassio4.local", 
          "port": "1883", 
          "user": "mqttapi", 
          "password": "supersecret",
          "tprefix": "",
        });
      });
      app.get('/loadP', function (req, res) {
        res.json([0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0]);
      });
      app.get("/screen", function(req,res){
        res.sendFile(path.join(__dirname, 'test/screen'+(read++ % 2)+'.pbm'));
      });
      app.get("/screenpbm", function(req,res){
        res.sendFile(path.join(__dirname, 'test/screenpbm'+(read++ % 2)+'.pbm'));
      });
      
    }
  }
};

module.exports = (env, argv) => {
//  console.log("Env:",process.env,"ArgV:",argv);
  genv = env;
  gargv = argv;

  if (argv.mode === 'development') {
    config.devtool = 'source-map';
    //    config.devtool = 'inline-source-map';    
    //    config.plugins.push( new webpack.SourceMapDevToolPlugin({}));    
  }

  if (argv.mode === 'production') {
    config.plugins.push(new CompressionPlugin({
      deleteOriginalAssets: true,
//      exclude: /\.html$/,
    }));
  }

  return config;
};


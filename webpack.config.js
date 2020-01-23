const path = require('path');
const CompressionPlugin = require('compression-webpack-plugin');
const TerserPlugin = require('terser-webpack-plugin');


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
  plugins: [],
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
              attrs: ['img:src', 'link:href', 'iframe:src'],
            }
          },
        ]
      },
      {
        test: /\.js$/,
        use: ["source-map-loader"],
        enforce: "pre"
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
    port: 9000,
    before: function (app, server, compiler) {
      //      var bodyParser = require('body-parser');
      //      app.use(bodyParser.json());
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
        res.json({ "wprg": [{ "day": 0, "prg": [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0] }, { "day": 1, "prg": [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0] }, { "day": 2, "prg": [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0] }, { "day": 3, "prg": [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0] }, { "day": 4, "prg": [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0] }, { "day": 5, "prg": [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0] }, { "day": 6, "prg": [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0] }] });
      });
    }
  }
};

module.exports = (env, argv) => {

  if (argv.mode === 'development') {
    config.devtool = 'source-map';
    //    config.devtool = 'inline-source-map';    
    //    config.plugins.push( new webpack.SourceMapDevToolPlugin({}));    
  }

  if (argv.mode === 'production') {
    config.plugins.push(new CompressionPlugin({
      deleteOriginalAssets: true,
      exclude: /\.html$/,
    }));
  }

  return config;
};


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
    publicPath: "/"
  },
  plugins: [],
  module: {
    rules: [
      { test: /\.css(\?mtime=\d+)?$/i, use: ['style-loader', 'css-loader'] },
      {
        test: /\.jpe?g$|\.ico$|\.gif$|\.png$|\.svg$|\.woff$|\.ttf$|\.wav$|\.mp3$/,
        loader: 'file-loader?name=[path][name].[ext]'  // <- retain original file name
      },
      {
        test: /\.html$/,
        use: ['file-loader', 
              { loader: 'extract-loader',
                options: {
                  name: '[path][name].[ext]',
                  publicPath: (url,resourcePath,context) => { 
                    console.log("----------------------------------------------------------");
                    console.log(url,resourcePath,context);
                    return '../'.repeat(path.relative(path.resolve('src/node'), context.context).split('/').length); 
                  },
                }
              },
              { 
                loader: 'html-loader',
                options: {
                  minimize: true,
                  attrs: ['img:src','link:href','iframe:src'],
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
    port: 9000
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


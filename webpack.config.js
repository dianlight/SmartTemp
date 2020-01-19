const path = require('path');
const CompressionPlugin = require('compression-webpack-plugin');
const TerserPlugin = require('terser-webpack-plugin');


var config = {
  entry: {
    main: './src/node/index.js'
  },
  output: {
    path: path.resolve(__dirname, 'data')
  },
  plugins: [],
  module: {
    rules: [
      { test: /\.css$/i, use: ['style-loader', 'css-loader'] },
      {
        test: /\.html$/,
        use: ['file-loader?name=[name].[ext]', 'extract-loader', 'html-loader?minimize=true'],
      }, 
      {
        test: /\.jpe?g$|\.ico$|\.gif$|\.png$|\.svg$|\.woff$|\.ttf$|\.wav$|\.mp3$/,
        loader: 'file-loader?name=[name].[ext]'  // <-- retain original file name
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
            deleteOriginalAssets: true
        }));
  }

  return config;
};


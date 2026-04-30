const allowNetworkAccess = process.env.LNG_DEVSERVER_EXPOSE === 'true'

module.exports = {
  devServer: {
    host: allowNetworkAccess ? '0.0.0.0' : '127.0.0.1',
    port: 9090,
    allowedHosts: allowNetworkAccess ? 'all' : 'auto',
  },
}

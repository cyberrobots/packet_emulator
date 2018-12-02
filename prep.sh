echo "start"
sudo ethtool -K enp0s25 tso off
sudo ethtool -K enx00e097005173 tso off
sudo ethtool -K enp0s25 gso off
sudo ethtool -K enx00e097005173 gso off
sudo ethtool -K enp0s25 tx off
sudo ethtool -K enx00e097005173 tx off
echo "done"

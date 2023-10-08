#!/bin/sh

pid=""
pid=`ps -ef | grep print_client_tls | grep niftyUnreal | awk '{print $2}'`

echo $pid
if [ -z "$pid" ]
then
      echo "\$pid is empty"
      echo "process down "
      /Users/VinayDipti/YellowBallMDFeed/websocketpp/examples/print_client_tls/client_p/print_client_tls niftyUnreal >> `date '+%H%M%S%_d%m%Y'`_mdunreal.log 2<&1 & 
else
      echo "\$pid is NOT empty"
fi
 currenttime=$(date +%H:%M)
if [[ "$currenttime" > "15:31" ]] || [[ "$currenttime" < "09:00" ]]; then
 echo "killing $pid"
     kill $pid 
fi

#!/bin/bash

#set -x

echo ' '
NO_PORTS=$(pmc -u 'GET PORT_DATA_SET' | egrep -c 'fffe')
NO_PORTS=$((NO_PORTS/2))
#echo 'Number of ports is: '$NO_PORTS
echo ' '
#echo "Bash version ${BASH_VERSION}..."
#echo ' '
#echo ' '



declare -a ARRAY
COUNT=0

ROWS=$(pmc -u 'GET PORT_DATA_SET' | grep portState | cut -f 16 -d ' ')

O=$IFS 
IFS=$(echo -en "\n\b") # set IFS to linebreak


for ROW in ${ROWS[@]}
do
  #echo $ROW row row
  ARRAY[COUNT]=$ROW
  #echo $COUNT
  #echo ${ARRAY[COUNT]}
  COUNT=$((COUNT+1))
done

IFS=$O #restore old IFS

########################
ROWS=$(pmc -u 'GET PORT_DATA_SET' | grep peerMeanPath | cut -f 8 -d ' ')

O=$IFS 
IFS=$(echo -en "\n\b") # set IFS to linebreak


for ROW in ${ROWS[@]}
do
  #echo $ROW row row
  ARRAY[COUNT]=$ROW
  #echo $COUNT
  #echo ${ARRAY[COUNT]}
  COUNT=$((COUNT+1))
done

IFS=$O #restore old IFS

########################
ROWS=$(pmc -u 'GET CLOCK_DESCRIPTION' | grep clockType | cut -f 14,15 -d ' ')

O=$IFS 
IFS=$(echo -en "\n\b") # set IFS to linebreak


for ROW in ${ROWS[@]}
do
  #echo $ROW row row
  ARRAY[COUNT]=$ROW
  #echo $COUNT
  #echo ${ARRAY[COUNT]}
  COUNT=$((COUNT+1))
done

IFS=$O #restore old IFS


########################
ROWS=$(pmc -u 'GET PORT_DATA_SET_NP' | grep neighborProp | cut -f 2 -d ' ')

O=$IFS 
IFS=$(echo -en "\n\b") # set IFS to linebreak


for ROW in ${ROWS[@]}
do
  #echo $ROW row row
  ARRAY[COUNT]=$ROW
  #echo $COUNT
  #echo ${ARRAY[COUNT]}
  COUNT=$((COUNT+1))
done

IFS=$O #restore old IFS


########################
ROWS=$(pmc -u 'GET PORT_DATA_SET_NP' | grep asCapable | cut -f 16 -d ' ')

O=$IFS 
IFS=$(echo -en "\n\b") # set IFS to linebreak


for ROW in ${ROWS[@]}
do
  #echo $ROW row row
  ARRAY[COUNT]=$ROW
  #echo $COUNT
  #echo ${ARRAY[COUNT]}
  COUNT=$((COUNT+1))
done

IFS=$O #restore old IFS


########################
ROWS=$(pmc -u 'GET TIME_STATUS_NP' | grep gmIdentity| cut -f 18 -d ' ')

O=$IFS 
IFS=$(echo -en "\n\b") # set IFS to linebreak


for ROW in ${ROWS[@]}
do
  #echo $ROW row row
  ARRAY[COUNT]=$ROW
  #echo $COUNT
  #echo ${ARRAY[COUNT]}
  COUNT=$((COUNT+1))
done

IFS=$O #restore old IFS


########################
ROWS=$(pmc -u 'GET TIME_STATUS_NP' | grep master_offset | cut -f 15 -d ' ')

O=$IFS 
IFS=$(echo -en "\n\b") # set IFS to linebreak


for ROW in ${ROWS[@]}
do
  #echo $ROW row row
  ARRAY[COUNT]=$ROW
  #echo $COUNT
  #echo ${ARRAY[COUNT]}
  COUNT=$((COUNT+1))
done

IFS=$O #restore old IFS


#echo ' '
#echo ${ARRAY[*]}

########################################################################


lowerLimit=0

TEMP=$(($NO_PORTS-1))
COUNT2=$(($NO_PORTS-$TEMP-1))
COUNT3=0
COUNT4=0
GMI=$((NO_PORTS*5))
MOFF=$((GMI+1))

#echo NO_PORTS $NO_PORTS
#echo COUNT2 $COUNT2 

for i in $(seq $lowerLimit $TEMP)
do
  echo '------------------------ PORT ${COUNT2}'
  echo -e "portState \t\t"${ARRAY[COUNT3]}
  COUNT3=$((COUNT3+NO_PORTS))
  echo -e "peerMeanPathDelay \t"${ARRAY[COUNT3]}"ns"
  COUNT3=$((COUNT3+NO_PORTS))
  echo -e "clockType \t\t"${ARRAY[COUNT3]}
  COUNT3=$((COUNT3+NO_PORTS))
  echo -e  "neighborPropDelayThresh "${ARRAY[COUNT3]}"ns"
  COUNT3=$((COUNT3+NO_PORTS))
  echo -e "asCapable \t\t"${ARRAY[COUNT3]}
  COUNT3=$((COUNT3+NO_PORTS))
  echo -e "gmIdentity \t\t"${ARRAY[GMI]}
  COUNT3=$((COUNT3+NO_PORTS))
  echo -e "master_offset \t\t"${ARRAY[MOFF]}
  echo ' '

  COUNT3=$((COUNT4+1))
  COUNT4=$((COUNT4+1))

  COUNT2=$((COUNT2+1))
done



#pmc -u 'GET PORT_DATA_SET' | grep portState | cut -f 16 -d ' '
#pmc -u 'GET PORT_DATA_SET' | grep peerMeanPath | cut -f 8 -d ' '
#pmc -u 'GET CLOCK_DESCRIPTION' | grep clockType | cut -f 14,15 -d ' '
#pmc -u 'GET TIME_STATUS_NP' | grep gmIdentity| cut -f 18 -d ' '
#pmc -u 'GET TIME_STATUS_NP' | grep master_offset | cut -f 15 -d ' '
#pmc -u 'GET PORT_DATA_SET_NP' | grep neighborProp | cut -f 2 -d ' '
#pmc -u 'GET PORT_DATA_SET_NP' | grep asCapable | cut -f 16 -d ' '




#!/usr/bin/env python
from numpy.distutils.exec_command import temp_file_name

import os
import sys
from multiprocessing import Pool

# This is a method which take as argument one simulation parameter, 
# forms the string you would usually give in a command line fashion,
# and runs it;
#from kiwi.decorators import signal_block
from pygccxml.declarations.cpptypes import restrict_t


def run_single_sim(sim_command):
    print "Now running:\n" + sim_command[0]
    os.chdir(g_pfp)
    os.system(sim_command[0])
    print "\nProcess no. " + sim_command[1] + " FINISHED:\n" + sim_command[0]


def main(argv):
    g_resultNameBase = argv[0]
    new_command = argv[1]   #apply to all
    g_group_command_16nodes = argv[2]
    g_group_command_36nodes = argv[3]
    g_group_command_100nodes = argv[4]

    def options(sim_type, resultName):
        return {
            # 0: 16-nodes, speed=0, pmin=2, pmax=5
            0:  g_common_commands + resultName + "-rwaypoint-16n-sp0 " + '--topology=6 --initialHopLimit=24 --maxHopLimit=24 --nWifi=16 --speed=0 --pauseTimeMin=2 --pauseTimeMax=5 ' ,
            # 1: 16-nodes, speed=1, pmin=2, pmax=5
            1:  g_common_commands + resultName + "-rwaypoint-16n-sp1 " + '--topology=6 --initialHopLimit=24 --maxHopLimit=24 --nWifi=16 --speed=1 --pauseTimeMin=2 --pauseTimeMax=5 ' ,
            #36-nodes
            # 2: 36-nodes, speed=0, pmin=2, pmax=5,
            2:  g_common_commands + resultName + "-rwaypoint-36n-sp0 " + '--topology=6 --initialHopLimit=54 --maxHopLimit=54 --nWifi=36 --speed=0 --pauseTimeMin=2 --pauseTimeMax=5 ' ,
            # 3: 36-nodes, speed=1, pmin=2, pmax=5
            3:  g_common_commands + resultName + "-rwaypoint-36n-sp1 " + '--topology=6 --initialHopLimit=54 --maxHopLimit=54 --nWifi=36 --speed=1 --pauseTimeMin=2 --pauseTimeMax=5 ' ,
            # 100-nodes
            # 4: 100-nodes, speed=0, pmin=2, pmax=5
            4:  g_common_commands + resultName + "-rwaypoint-100n-sp0 " + '--topology=6 --initialHopLimit=150 --maxHopLimit=150 --nWifi=100 --RETRANS_TIMER_NDAD=3500 --FrwDelay=200 --speed=0 --pauseTimeMin=2 --pauseTimeMax=5 ' ,
            # 5: 100-nodes, speed=1,  pmin=2, pmax=5
            5:  g_common_commands + resultName + "-rwaypoint-100n-sp1 " + '--topology=6 --initialHopLimit=150 --maxHopLimit=150 --nWifi=100 --RETRANS_TIMER_NDAD=3500 --FrwDelay=200 --speed=1 --pauseTimeMin=2 --pauseTimeMax=5 ' ,
        }.get(sim_type, str('ERROR'))    # "ERROR" is default if x not found


    log_folder = g_pfp + "simul_logs/" + g_resultNameBase + "/"
    os.system("mkdir -p " + log_folder)

     # This is the list with the parameters which will change in your simulations; it is split by the multiprocessing library and fed to the Pool, which initiates the simulations on as many threads you want.
    sim_command_list = []

    #for mprp_delay=1
    resultName = g_resultNameBase + "-mprpD_1"
    for x in range(0, 6):
        log_file = log_folder + "log_" + g_resultNameBase + "_" + str(x) + ".out"
        # touch the dump file where the log will be written
        os.system("touch " + log_file)
        if x < 2:
            temp_command = options(x,resultName) + new_command + " " + "--resultGroupName=" + g_resultNameBase + " --MPRP_DELAY=1 --MprStartDelay=1 " + g_group_command_16nodes + '" > ' + log_file + " 2>&1"
        elif x < 4:
            temp_command = options(x,resultName) + new_command + " " + "--resultGroupName=" + g_resultNameBase + " --MPRP_DELAY=1 --MprStartDelay=1 " + g_group_command_36nodes + '" > ' + log_file + " 2>&1"
        else:
            temp_command = options(x,resultName) + new_command + " " + "--resultGroupName=" + g_resultNameBase + " --MPRP_DELAY=1 --MprStartDelay=1 " + g_group_command_100nodes + '" > ' + log_file + " 2>&1"
        sim_command_list.append([temp_command, str(x)])

    #for mprp_delay=0.5
    resultName = g_resultNameBase + "-mprpD_0_5"
    for x in range(0, 6):
        log_file = log_folder + "log_" + g_resultNameBase + "_" + str(x+6) + ".out"
        # touch the dump file where the log will be written
        os.system("touch " + log_file)
        if x < 2:
            temp_command = options(x,resultName) + new_command + " " + "--resultGroupName=" + g_resultNameBase + " --MPRP_DELAY=0.5 --MprStartDelay=0.5 " + g_group_command_16nodes + '" > ' + log_file + " 2>&1"
        elif x < 4:
            temp_command = options(x,resultName) + new_command + " " + "--resultGroupName=" + g_resultNameBase + " --MPRP_DELAY=0.5 --MprStartDelay=0.5 " + g_group_command_36nodes + '" > ' + log_file + " 2>&1"
        else:
            temp_command = options(x,resultName) + new_command + " " + "--resultGroupName=" + g_resultNameBase + " --MPRP_DELAY=0.5 --MprStartDelay=0.5 " + g_group_command_100nodes + '" > ' + log_file + " 2>&1"
        sim_command_list.append([temp_command, str(x+6)])

    # Specify here how many processes/threads you want
    pool = Pool(processes=7)  # start X workers processes

    # Give here the name of your function which can run one simulation with a given parameter, and a list of parameters which will be split among the threads.
    pool.map(run_single_sim, sim_command_list)


if __name__ == "__main__":
    # A few global parameters related to folder structure
    g_pfp = "/home/monika/Workspaces/ns-3-update-nd++/ns-3.18/"
    g_common_commands = './waf --run "scratch/ipv6-ndpp-testsuite --verbose=-1 --ndType=4 --wifiStd=2 --wifiMode=2 --noOfRuns=10 --initiatorNo=-1 --targetNo=-1 --ctrlNoOfRuns=0 --nDADcount=3 --frwCount=1 --dadReplyCount=1 --DadDelay=1 --netAnim=0 --multiExpDupNoOfRuns=50 --channelType=0 --resultFileSuffix='
    #--randomNodesNo=4 --noOfDupl=4 --noOfSameDupl=0 --rnNoOfRuns=10 --newNodes=1 --newAutoAddresses=1

    main(sys.argv[1:])

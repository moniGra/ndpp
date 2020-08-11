#!/usr/bin/env python
from numpy.distutils.exec_command import temp_file_name

import os
import sys
from multiprocessing import Pool

# This is a method which take as argument one simulation parameter, 
# forms the string you would usually give in a command line fashion,
# and runs it;
#from kiwi.decorators import signal_block


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

    def options(sim_type):
        return {
            # 0: 16-nodes, speed=0, pmin=5, pmax=7
            0:  g_common_commands + g_resultNameBase + "-rwaypoint-16n-sp0-pause5,7 " + '--topology=6 --initialHopLimit=24 --maxHopLimit=24 --nWifi=16 --speed=0 --pauseTimeMin=5 --pauseTimeMax=7 ' ,
            # 1: 16-nodes, speed=0,  pmin=2, pmax=3
            1:  g_common_commands + g_resultNameBase + "-rwaypoint-16n-sp0-pause2,3 " + '--topology=6 --initialHopLimit=24 --maxHopLimit=24 --nWifi=16 --speed=0 --pauseTimeMin=2 --pauseTimeMax=3 ' ,
            # 2: 16-nodes, speed=0,  pmin=8, pmax=10
            2:  g_common_commands + g_resultNameBase + "-rwaypoint-16n-sp0-pause8,10 " + '--topology=6 --initialHopLimit=24 --maxHopLimit=24 --nWifi=16 --speed=0 --pauseTimeMin=8 --pauseTimeMax=10 ' ,
            # 3: 16-nodes, speed=1, pmin=5, pmax=7
            3:  g_common_commands + g_resultNameBase + "-rwaypoint-16n-sp1-pause5,7 " + '--topology=6 --initialHopLimit=24 --maxHopLimit=24 --nWifi=16 --speed=1 --pauseTimeMin=5 --pauseTimeMax=7 ' ,
            # 4: 16-nodes, speed=1,  pmin=2, pmax=3
            4:  g_common_commands + g_resultNameBase + "-rwaypoint-16n-sp1-pause2,3 " + '--topology=6 --initialHopLimit=24 --maxHopLimit=24 --nWifi=16 --speed=1 --pauseTimeMin=2 --pauseTimeMax=3 ' ,
            # 5: 16-nodes, speed=1,  pmin=8, pmax=10
            5:  g_common_commands + g_resultNameBase + "-rwaypoint-16n-sp1-pause8,10 " + '--topology=6 --initialHopLimit=24 --maxHopLimit=24 --nWifi=16 --speed=1 --pauseTimeMin=8 --pauseTimeMax=10 ' ,
            #36-nodes
            # 6: 36-nodes, speed=0, pmin=5, pmax=7
            6:  g_common_commands + g_resultNameBase + "-rwaypoint-36n-sp0-pause5,7 " + '--topology=6 --initialHopLimit=54 --maxHopLimit=54 --nWifi=36 --speed=0 --pauseTimeMin=5 --pauseTimeMax=7 ' ,
            # 7: 36-nodes, speed=0,  pmin=2, pmax=3
            7:  g_common_commands + g_resultNameBase + "-rwaypoint-36n-sp0-pause2,3 " + '--topology=6 --initialHopLimit=54 --maxHopLimit=54 --nWifi=36 --speed=0 --pauseTimeMin=2 --pauseTimeMax=3 ' ,
            # 8: 36-nodes, speed=0,  pmin=8, pmax=10
            8:  g_common_commands + g_resultNameBase + "-rwaypoint-36n-sp0-pause8,10 " + '--topology=6 --initialHopLimit=54 --maxHopLimit=54 --nWifi=36 --speed=0 --pauseTimeMin=8 --pauseTimeMax=10 ' ,
            # 9: 36-nodes, speed=1, pmin=5, pmax=7
            9:  g_common_commands + g_resultNameBase + "-rwaypoint-36n-sp1-pause5,7 " + '--topology=6 --initialHopLimit=54 --maxHopLimit=54 --nWifi=36 --speed=1 --pauseTimeMin=5 --pauseTimeMax=7 ' ,
            # 10: 36-nodes, speed=1,  pmin=2, pmax=3
            10:  g_common_commands + g_resultNameBase + "-rwaypoint-36n-sp1-pause2,3 " + '--topology=6 --initialHopLimit=54 --maxHopLimit=54 --nWifi=36 --speed=1 --pauseTimeMin=2 --pauseTimeMax=3 ' ,
            # 11: 36-nodes, speed=1,  pmin=8, pmax=10
            11:  g_common_commands + g_resultNameBase + "-rwaypoint-36n-sp1-pause8,10 " + '--topology=6 --initialHopLimit=54 --maxHopLimit=54 --nWifi=36 --speed=1 --pauseTimeMin=8 --pauseTimeMax=10 ' ,
            # 100-nodes
            # 12: 100-nodes, speed=0, pmin=5, pmax=7
            12:  g_common_commands + g_resultNameBase + "-rwaypoint-100n-sp0-pause5,7 " + '--topology=6 --initialHopLimit=150 --maxHopLimit=150 --nWifi=100 --RETRANS_TIMER_NDAD=3500 --FrwDelay=200 --speed=0 --pauseTimeMin=5 --pauseTimeMax=7 ' ,
            # 13: 100-nodes, speed=0,  pmin=2, pmax=3
            13:  g_common_commands + g_resultNameBase + "-rwaypoint-100n-sp0-pause2,3 " + '--topology=6 --initialHopLimit=150 --maxHopLimit=150 --nWifi=100 --RETRANS_TIMER_NDAD=3500 --FrwDelay=200 --speed=0 --pauseTimeMin=2 --pauseTimeMax=3 ' ,
            # 14: 100-nodes, speed=0,  pmin=8, pmax=10
            14:  g_common_commands + g_resultNameBase + "-rwaypoint-100n-sp0-pause8,10 " + '--topology=6 --initialHopLimit=150 --maxHopLimit=150 --nWifi=100 --RETRANS_TIMER_NDAD=3500 --FrwDelay=200 --speed=0 --pauseTimeMin=8 --pauseTimeMax=10 ' ,
            # 15: 100-nodes, speed=1, pmin=5, pmax=7
            15:  g_common_commands + g_resultNameBase + "-rwaypoint-100n-sp1-pause5,7 " + '--topology=6 --initialHopLimit=150 --maxHopLimit=150 --nWifi=100 --RETRANS_TIMER_NDAD=3500 --FrwDelay=200 --speed=1 --pauseTimeMin=5 --pauseTimeMax=7 ' ,
            # 16: 100-nodes, speed=1,  pmin=2, pmax=3
            16:  g_common_commands + g_resultNameBase + "-rwaypoint-100n-sp1-pause2,3 " + '--topology=6 --initialHopLimit=150 --maxHopLimit=150 --nWifi=100 --RETRANS_TIMER_NDAD=3500 --FrwDelay=200 --speed=1 --pauseTimeMin=2 --pauseTimeMax=3 ' ,
            # 17: 100-nodes, speed=1,  pmin=8, pmax=10
            17:  g_common_commands + g_resultNameBase + "-rwaypoint-100n-sp1-pause8,10 " + '--topology=6 --initialHopLimit=150 --maxHopLimit=150 --nWifi=100 --RETRANS_TIMER_NDAD=3500 --FrwDelay=200 --speed=1 --pauseTimeMin=8 --pauseTimeMax=10 ' ,
        }.get(sim_type, str('ERROR'))    # "ERROR" is default if x not found


    log_folder = g_pfp + "simul_logs/" + g_resultNameBase + "/"
    os.system("mkdir -p " + log_folder)

     # This is the list with the parameters which will change in your simulations; it is split by the multiprocessing library and fed to the Pool, which initiates the simulations on as many threads you want.
    sim_command_list = []

    for x in range(0, 18):
        log_file = log_folder + "log_" + g_resultNameBase + "_" + str(x) + ".out"
        # touch the dump file where the log will be written
        os.system("touch " + log_file)
        if x < 6:
            temp_command = options(x) + new_command + " " + "--resultGroupName=" + g_resultNameBase + " " + g_group_command_16nodes + ' " > ' + log_file + " 2>&1"
        elif x < 12:
            temp_command = options(x) + new_command + " " + "--resultGroupName=" + g_resultNameBase + " " + g_group_command_36nodes + ' " > ' + log_file + " 2>&1"
        else:
            temp_command = options(x) + new_command + " " + "--resultGroupName=" + g_resultNameBase + " " + g_group_command_100nodes + ' " > ' + log_file + " 2>&1"
        sim_command_list.append([temp_command, str(x)])

    # Specify here how many processes/threads you want
    pool = Pool(processes=7)  # start X workers processes

    # Give here the name of your function which can run one simulation with a given parameter, and a list of parameters which will be split among the threads.
    pool.map(run_single_sim, sim_command_list)


if __name__ == "__main__":
    # A few global parameters related to folder structure
    g_pfp = "/home/monika/Workspaces/ns-3-update-nd++/ns-3.18/"
    g_common_commands = './waf --run "scratch/ipv6-ndpp-testsuite --verbose=-1 --ndType=4 --wifiStd=2 --wifiMode=2 --noOfRuns=10 --initiatorNo=-1 --targetNo=-1 --ctrlNoOfRuns=0 --nDADcount=3 --frwCount=1 --dadReplyCount=1 --DadDelay=1 --netAnim=0 --multiExpDupNoOfRuns=50 --channelType=1 --resultFileSuffix='
    #--randomNodesNo=4 --noOfDupl=4 --noOfSameDupl=0 --rnNoOfRuns=10 --newNodes=1 --newAutoAddresses=1

    main(sys.argv[1:])

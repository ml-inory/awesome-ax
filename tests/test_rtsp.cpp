//
// Created by yangrongzhao on 2024/1/3.
//
#include "nodes/RTSPPullNode.hpp"
#include "nodes/RTSPPushNode.hpp"

#include <cstdio>
#include <csignal>
#include <memory>
#include <cstdlib>
#include <thread>
#include <fstream>

#include "json/json.h"

using namespace ax;

volatile int g_isRunning = 1;

static void EXIT() {
    AX_POOL_Exit();
    AX_SYS_Deinit();
}

extern "C" void __sigExit(int iSigNo) {
    g_isRunning = 0;
    printf("Interrupt\n");
    sleep(1);
    return;
}

int main(int argc, char** argv) {
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, __sigExit);

    EXIT();

    int ret = AX_SUCCESS;
    ret = AX_SYS_Init();
    if (ret != AX_SUCCESS) {
        printf("AX_SYS_Init failed! ret=0x%x\n", ret);
        return -1;
    }

    AX_POOL_FLOORPLAN_T stPoolCfg;
    memset(&stPoolCfg, 0, sizeof(AX_POOL_FLOORPLAN_T));
    stPoolCfg.CommPool[0].BlkSize = 1920 * 1080 * 3 / 2;
    stPoolCfg.CommPool[0].BlkCnt = 10;
    ret = AX_POOL_SetConfig(&stPoolCfg);
    if (ret != AX_SUCCESS) {
        printf("AX_POOL_SetConfig failed! ret=0x%x\n", ret);
        EXIT();
        return -1;
    }

    ret = AX_POOL_Init();
    if (ret != AX_SUCCESS) {
        printf("AX_POOL_Init failed! ret=0x%x\n", ret);
        EXIT();
        return -1;
    }

    Json::Value config;
    std::ifstream ifs("test_rtsp.json");
    ifs >> config;

    auto pull_node = std::make_shared<RTSPPullNode>();
    if (AX_SUCCESS != pull_node->Init(config)) {
        printf("pull_node init failed!\n");
        EXIT();
        return -1;
    }

    auto push_node = std::make_shared<RTSPPushNode>();
    if (AX_SUCCESS != push_node->Init(config)) {
        printf("push_node init failed!\n");
        EXIT();
        return -1;
    }

    pull_node->Connect(push_node);

    pull_node->Start();
    push_node->Start();

    std::thread pull_thread(&Node::Run, pull_node);
    pull_thread.detach();

    std::thread push_thread(&Node::Run, push_node);
    push_thread.detach();

    while (g_isRunning) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    printf("Stop\n");

    pull_node->Stop();
    push_node->Stop();
    sleep(3);


    EXIT();
    return 0;
}
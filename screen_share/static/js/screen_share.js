'use strict';

// 获取本地和远程视频元素
const localVideo = document.getElementById('localVideo');
const remoteVideo = document.getElementById('remoteVideo');

// 获取控制按钮元素
const startPushBtn = document.getElementById('btnStartPush');
const stopPushBtn = document.getElementById('btnStopPush');
const startPullBtn = document.getElementById('btnStartPull');
const stopPullBtn = document.getElementById('btnStopPull');

// 为按钮添加点击事件监听器
startPushBtn.addEventListener('click', startPush);
stopPushBtn.addEventListener('click', stopPush);
startPullBtn.addEventListener('click', startPull);
stopPullBtn.addEventListener('click', stopPull);

// WebRTC配置对象（空配置）
const config = {};

// 创建offer时的选项配置 - 不接收音频和视频
const offerOptions = {
    offerToReceiveAudio: false,
    offerToReceiveVideo: false
};

// 创建两个RTCPeerConnection实例
// pc1: 本地端（推送流）
// pc2: 远程端（拉取流）
let pc1 = new RTCPeerConnection(config);
let pc2 = new RTCPeerConnection(config);
let remoteStream; // 存储远程流的变量

/**
 * 开始推送屏幕流
 */
function startPush() {
    console.log("start push stream");

    // 发送消息请求屏幕共享（可能是与浏览器扩展通信）
    window.postMessage({type: 'SS_UI_REQUEST', text: "push"}, '*');
}

/**
 * 开始拉取远程流
 */
function startPull() {
    console.log("start pull stream");
    
    // 将远程视频元素的源设置为远程流
    remoteVideo.srcObject = remoteStream;

    // 创建answer响应
    pc2.createAnswer().then(
        onCreateAnswerSuccess, // 成功回调
        onCreateSessionDescriptionError // 错误回调
    );
}

/**
 * 停止推送流
 */
function stopPush() {
    console.log("pc1 stop push stream");

    // 关闭pc1连接
    if (pc1) {
        pc1.close();
        pc1 = null;
    }

    // 清空本地视频源
    localVideo.srcObject = null;
}

/**
 * 停止拉取流
 */
function stopPull() {
    console.log("pc2 stop pull stream");

    // 关闭pc2连接
    if (pc2) {
        pc2.close();
        pc2 = null;
    }

    // 清空远程视频源
    remoteVideo.srcObject = null;
}

/**
 * 成功创建answer后的处理
 * @param {RTCSessionDescription} desc - answer描述
 */
function onCreateAnswerSuccess(desc) {
    console.log('answer from pc2: \n' + desc.sdp);
    
    // pc2设置本地描述
    console.log('pc2 set local description start');
    pc2.setLocalDescription(desc).then(
        function() {
            onSetLocalSuccess(pc2);
        },
        onSetSessionDescriptionError
    );

    // pc1设置远程描述（交换SDP）
    pc1.setRemoteDescription(desc).then(
        function() {
            onSetRemoteSuccess(pc1);
        },
        onSetSessionDescriptionError
    );
}

/**
 * 监听来自窗口的消息事件（用于屏幕共享）
 */
window.addEventListener('message', function(event) {
    // 安全检查：确保消息来源与当前页面同源
    if (event.origin != window.location.origin) {
        return;
    }

    // 处理屏幕共享对话框的结果
    if (event.data.type && event.data.type === 'SS_DIALOG_SUCCESS') {
        console.log("用户同意屏幕共享, streamId: " + event.data.streamId);
        startScreenStreamFrom(event.data.streamId); // 开始屏幕流
    } else if (event.data.type && event.data.type === 'SS_DIALOG_CANCEL') {
        console.log("用户取消屏幕共享");
    }
});

/**
 * 根据流ID开始屏幕流
 * @param {string} streamId - 屏幕流的ID
 */
function startScreenStreamFrom(streamId) {
    // 媒体约束配置，指定使用桌面作为视频源
    const constraints = {
        audio: false, // 不捕获音频
        video: {
            mandatory: {
                chromeMediaSource: 'desktop', // Chrome特定的桌面捕获源
                chromeMediaSourceId: streamId, // 流ID
                maxWidth: window.screen.width, // 最大宽度为屏幕宽度
                maxHeight: window.screen.height // 最大高度为屏幕高度
            }
        }
    };

    // 获取用户媒体流
    navigator.mediaDevices.getUserMedia(constraints)
        .then(handleSuccess) // 成功回调
        .catch(handleError); // 错误回调
}

/**
 * 成功获取媒体流后的处理
 * @param {MediaStream} stream - 获取到的媒体流
 */
function handleSuccess(stream) {
    console.log("get screen stream success");
    localVideo.srcObject = stream; // 设置本地视频源
    
    // 设置pc1的ICE连接状态变化回调
    pc1.oniceconnectionstatechange = function(e) {
        onIceStateChange(pc1, e);
    };

    // 设置pc1的ICE候选收集回调
    pc1.onicecandidate = function(e) {
        onIceCandidate(pc1, e);
    }
    
    // 将流添加到pc1
    pc1.addStream(stream);

    // 创建offer
    pc1.createOffer(offerOptions).then(
        onCreateOfferSuccess, // 成功回调
        onCreateSessionDescriptionError // 错误回调
    );
}

/**
 * 获取PeerConnection的标识字符串
 * @param {RTCPeerConnection} pc - PeerConnection实例
 * @returns {string} 标识字符串
 */
function getPc(pc) {
    return pc == pc1 ? 'pc1' : 'pc2';
}

/**
 * 成功创建offer后的处理
 * @param {RTCSessionDescription} desc - offer描述
 */
function onCreateOfferSuccess(desc) {
    console.log('offer from pc1: \n' + desc.sdp);

    // pc1设置本地描述
    console.log('pc1 set local description start');
    pc1.setLocalDescription(desc).then(
        function() {
            onSetLocalSuccess(pc1);
        },
        onSetSessionDescriptionError
    );

    // 设置pc2的回调函数
    pc2.oniceconnectionstatechange = function(e) {
        onIceStateChange(pc2, e);
    }

    pc2.onicecandidate = function(e) {
        onIceCandidate(pc2, e);
    }

    // 设置pc2的流添加回调
    pc2.onaddstream = function(e) {
        console.log('pc2 receive stream, stream_id: ' + e.stream.id);
        remoteStream = e.stream; // 保存远程流
    }

    // pc2设置远程描述（交换SDP）
    pc2.setRemoteDescription(desc).then(
        function() {
            onSetRemoteSuccess(pc2);
        },
        onSetSessionDescriptionError
    );
}

/**
 * 成功设置本地描述后的回调
 * @param {RTCPeerConnection} pc - PeerConnection实例
 */
function onSetLocalSuccess(pc) {
    console.log(getPc(pc) + ' set local success');
}

/**
 * 成功设置远程描述后的回调
 * @param {RTCPeerConnection} pc - PeerConnection实例
 */
function onSetRemoteSuccess(pc) {
    console.log(getPc(pc) + ' set remote success');
}

/**
 * 创建会话描述失败的回调
 * @param {Error} err - 错误对象
 */
function onCreateSessionDescriptionError(err) {
    console.log('create session description error: ' + err.toString());
}

/**
 * 设置会话描述失败的回调
 * @param {Error} err - 错误对象
 */
function onSetSessionDescriptionError(err) {
    console.log('set session description error: ' + err.toString());
}

/**
 * ICE连接状态变化的回调
 * @param {RTCPeerConnection} pc - PeerConnection实例
 * @param {Event} e - 事件对象
 */
function onIceStateChange(pc, e) {
    console.log(getPc(pc) + ' ice state change: ' + pc.iceConnectionState);
}

/**
 * 获取另一个PeerConnection实例
 * @param {RTCPeerConnection} pc - 当前PeerConnection实例
 * @returns {RTCPeerConnection} 另一个PeerConnection实例
 */
function getOther(pc) {
    return pc == pc1 ? pc2 : pc1;
}

/**
 * ICE候选收集的回调
 * @param {RTCPeerConnection} pc - PeerConnection实例
 * @param {RTCPeerConnectionIceEvent} e - ICE事件
 */
function onIceCandidate(pc, e) {
    console.log(getPc(pc) + ' get new ice candidate: ' + 
        (e.candidate ? e.candidate.candidate : '(null)'));

    // 将ICE候选添加到另一个PeerConnection
    getOther(pc).addIceCandidate(e.candidate).then(
        function() {
            console.log(getPc(getOther(pc)) + ' add ice candidate success');
        },
        function(err) {
            console.log(getPc(getOther(pc)) + ' add ice candidate error: ' + err.toString());
        }
    );
}

/**
 * 处理获取媒体流失败的回调
 * @param {Error} err - 错误对象
 */
function handleError(err) {
    console.log("get screen stream error: " + err.toString());
}
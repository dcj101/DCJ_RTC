'use strict';

// https://chromeextensionsdocs.appspot.com/apps/content_scripts#host-page-communication
//   - 'content_script' and execution env are isolated from each other
//   - In order to communicate we use the DOM (window.postMessage)
//
// app.js            |        |content-script.js |      |background.js
// window.postMessage|------->|port.postMessage  |----->| port.onMessage
//                   | window |                  | port |
// webkitGetUserMedia|<------ |window.postMessage|<-----| port.postMessage
//

// var port = chrome.runtime.connect(chrome.runtime.id);

// console.log('port', port);

function createConnection() {
  try {
      // 检查运行时是否可用
      if (!chrome.runtime || !chrome.runtime.id) {
          throw new Error("Chrome runtime not available");
      }
      
      // 尝试建立连接
      const port = chrome.runtime.connect(chrome.runtime.id);
      
      // 检查连接是否立即失败
      if (chrome.runtime.lastError) {
          throw new Error(chrome.runtime.lastError.message);
      }
      
      // 监听断开事件
      port.onDisconnect.addListener(() => {
          if (chrome.runtime.lastError) {
              console.error("端口断开:", chrome.runtime.lastError.message);
          } else {
              console.log("端口正常断开");
          }
      });
      
      return port;
      
  } catch (error) {
      console.error("创建连接失败:", error);
      return null;
  }
}

// 使用示例
const port = createConnection();
if (port) {
  // 连接成功
  port.postMessage({ type: 'TEST' });
} else {
  // 连接失败，需要处理
  console.error("无法连接到background script");
}

port.onMessage.addListener(function(msg) {
  window.postMessage(msg, '*');
});

window.addEventListener('message', function(event) {
  // We only accept messages from ourselves
  if (event.source !== window) {
    return;
  }

  if (event.data.type && ((event.data.type === 'SS_UI_REQUEST') ||
    (event.data.type === 'SS_UI_CANCEL'))) {
    port.postMessage(event.data);
  }
}, false);

window.postMessage({
  type: 'SS_PING',
  text: 'start'
}, '*');

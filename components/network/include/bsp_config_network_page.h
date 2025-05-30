/*
 * @Author: xingnian j_xingnian@163.com
 * @Date: 2025-05-30 20:25:03
 * @LastEditors: 星年 && j_xingnian@163.com
 * @LastEditTime: 2025-05-30 21:14:01
 * @FilePath: \ESP32-ChunFeng\components\network\include\bsp_config_network_page.h
 * @Description: 优化后的WiFi配网页面
 */
#pragma once

static const char* CONFIG_PAGE_HTML = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>春风WiFi配置</title>
    <style>
        body {
            font-family: 'Arial', sans-serif;
            background-color: #f4f4f4;
            color: #333;
            margin: 0;
            padding: 0;
            display: flex;
            flex-direction: column;
            justify-content: center;
            align-items: center;
            min-height: 100vh;
        }
        h2 {
            color: #4CAF50;
            margin-bottom: 20px;
        }
        input[type="text"], input[type="password"], button {
            width: 100%;
            padding: 10px;
            margin: 10px 0;
            font-size: 16px;
            border: 1px solid #ccc;
            border-radius: 5px;
            box-sizing: border-box;
        }
        button {
            background-color: #4CAF50;
            color: white;
            border: none;
            cursor: pointer;
            transition: background-color 0.3s;
        }
        button:disabled {
            background-color: #aaa;
            cursor: not-allowed;
        }
        button:hover:enabled {
            background-color: #45a049;
        }
        .connection-status {
            width: 100%;
            max-width: 350px;
            padding: 10px;
            margin-top: 20px;
            background-color: #fff;
            border-radius: 8px;
            box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
            text-align: center;
            word-break: break-all;
        }
        .connection-status p {
            margin: 0;
            color: #333;
        }
        .wifi-list-container {
        width: 100%;
        max-width: 350px;
        height: 220px;
        overflow-y: auto;
        background-color: #fff;
        border-radius: 10px;
        box-shadow: 0 2px 12px rgba(0,0,0,0.08);
        margin-top: 20px;
        padding: 10px;
    }
    .wifi-item {
        display: flex;
        align-items: center;
        justify-content: space-between;
        background: linear-gradient(90deg, #f8fafc 80%, #e3f2fd 100%);
        border-radius: 7px;
        box-shadow: 0 1px 4px rgba(76,175,80,0.06);
        margin-bottom: 12px;
        padding: 8px 12px;
        transition: box-shadow 0.2s, background 0.2s;
        cursor: pointer;
    }
    .wifi-item:hover {
        background: #e3f2fd;
        box-shadow: 0 2px 8px rgba(33,150,243,0.10);
    }
    .wifi-ssid {
        font-size: 15px;
        color: #333;
        font-weight: 500;
        max-width: 120px;
        overflow: hidden;
        text-overflow: ellipsis;
        white-space: nowrap;
        display: inline-block;
        vertical-align: middle;
    }
    .wifi-ssid[title]:hover::after {
        content: attr(title);
        position: absolute;
        background: #222;
        color: #fff;
        padding: 2px 8px;
        border-radius: 4px;
        font-size: 13px;
        left: 0;
        top: 100%;
        z-index: 10;
        white-space: normal;
    }
    .wifi-signal {
        margin-right: 10px;
        font-size: 20px;
        width: 24px;
        text-align: center;
    }
    .wifi-item button {
        background-color: #2196F3;
        width: 70px;
        margin-left: 10px;
        border-radius: 5px;
        font-size: 15px;
    }
</style>
    <script>
        window.onload = function() {
            scanWiFi();
            updateConnection();
        }
        // 扫描WiFi并更新列表
        function scanWiFi() {
            let btn = document.getElementById('scanBtn');
            btn.disabled = true;
            btn.innerText = '正在扫描...';
            fetch('/scan').then(r => r.json()).then(list => {
                // 按信号强度排序
                list.sort((a, b) => (b.rssi || 0) - (a.rssi || 0));
                let html = '';
                list.forEach(w => {
                    // 信号强度图标
                    let rssi = w.rssi || -100;
                    let bars = 1;
                    if(rssi > -50) bars = 4;
                    else if(rssi > -60) bars = 3;
                    else if(rssi > -70) bars = 2;
                    let color = bars >= 3 ? '#4CAF50' : (bars == 2 ? '#FFC107' : '#F44336');
                    let icon = `<svg class="wifi-signal" width="24" height="20" viewBox="0 0 24 20">
                        <g>
                            <path d="M12 18c-1.1 0-2-.9-2-2h4c0 1.1-.9 2-2 2z" fill="${bars>=1?color:'#ccc'}"/>
                            <path d="M7 14c0-2.8 2.2-5 5-5s5 2.2 5 5h-2c0-1.7-1.3-3-3-3s-3 1.3-3 3H7z" fill="${bars>=2?color:'#eee'}"/>
                            <path d="M2 10c0-5.5 4.5-10 10-10s10 4.5 10 10h-2c0-4.4-3.6-8-8-8s-8 3.6-8 8H2z" fill="${bars>=3?color:'#eee'}"/>
                        </g>
                    </svg>`;
                    html += `<div class="wifi-item">
                        ${icon}
                        <span class="wifi-ssid" title="${w.ssid}">${w.ssid}</span>
                        <button onclick="selectSSID('${w.ssid}')">选择</button>
                    </div>`;
                });
                document.getElementById('wifiList').innerHTML = html;
                btn.disabled = false;
                btn.innerText = '扫描WiFi';
            }).catch(()=>{
                btn.disabled = false;
                btn.innerText = '扫描WiFi';
            });
        }
        // 选择WiFi
        function selectSSID(ssid) {
            document.getElementById('ssid').value = ssid;
        }
        // 获取当前连接状态并显示
        function updateConnection() {
            fetch('/info').then(r => r.text()).then(t => {
                // 只显示SSID部分
                let ssid = t.replace(/^SSID:\s*/i, '').replace(/,.*/,'');
                if(ssid && ssid !== '未连接') {
                    document.getElementById('currentConnection').innerText = '当前连接: ' + ssid;
                } else {
                    document.getElementById('currentConnection').innerText = '当前未连接WiFi';
                }
            });
        }
        // 删除WiFi设置
        function deleteWiFi() {
            fetch('/delete', { method: 'POST' }).then(r => r.text()).then(t => {
                alert(t);
                updateConnection();
            });
        }
        // 拦截表单提交，AJAX方式连接WiFi
        function submitConnect(event) {
            event.preventDefault();
            let ssid = document.getElementById('ssid').value;
            let password = document.querySelector('input[name="password"]').value;
            let data = `ssid=${encodeURIComponent(ssid)}&password=${encodeURIComponent(password)}`;
            fetch('/connect', {
                method: 'POST',
                headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                body: data
            }).then(r => r.text()).then(t => {
                alert(t);
                updateConnection();
            });
        }
    </script>
</head>
<body>
    <h2>春风WiFi配置</h2>
    <div class="connection-status">
        <p id="currentConnection">加载中...</p>
    </div>
    <form id="wifiForm" style="width: 100%; max-width: 350px;">
        <input id="ssid" name="ssid" type="text" placeholder="输入SSID" required><br>
        <input name="password" type="password" placeholder="输入密码" required><br>
        <button type="submit">连接</button>
    </form>
    <div style="width: 100%; max-width: 350px; text-align: center;">
        <button id="scanBtn" onclick="scanWiFi();return false;">扫描WiFi</button>
        <button onclick="deleteWiFi();return false;">删除WiFi</button>
    </div>
    <div class="wifi-list-container">
        <div id="wifiList"></div>
    </div>
    <script>
        // 绑定表单提交事件
        document.getElementById('wifiForm').onsubmit = submitConnect;
    </script>
</body>
</html>
)rawliteral";
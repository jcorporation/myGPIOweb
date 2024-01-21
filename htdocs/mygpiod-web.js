let socket = null;

function getUri(proto) {
    const protocol = proto === 'ws'
        ? window.location.protocol === 'https:'
            ? 'wss:'
            : 'ws:'
        : window.location.protocol;
    return protocol + '//' + window.location.hostname +
        (window.location.port !== '' ? ':' + window.location.port : '');
}

function setError(msg) {
    document.getElementById('lastError').textContent = msg;
}

function socketConnect() {
    socket = new WebSocket(getUri('ws') + '/ws');
    socket.onopen = function() {
        document.getElementById('websocketState').textContent = 'Connected';
    }
    socket.onclose = function() {
        socket = null;
        document.getElementById('websocketState').textContent = 'Disconnected';
    }
    socket.onmessage = function(msg) {
        const msgRow = document.createElement('pre');
        msgRow.textContent = msg.data;
        document.getElementById('events').appendChild(msgRow);
    }
}

function getGPIOs() {
    const gpiosEl = document.getElementById('gpios');
    gpiosEl.textContent = '';
    httpRequest('GET', '/api/gpio', function(data) {
        for (const i = 0; i < data.entries; i++) {
            const tr = document.createElement('tr');
            for (const k of ['gpio', 'direction', 'value']) {
                const td = document.createElement('td');
                td.textContent = data.data[k];
                tr.appendChild(td);
            }
        }
    });
}

async function httpRequest(method, path, callback) {
    const uri = getUri('http') + path;
    let response = null;
    try {
        response = await fetch(uri, {
            method: method,
            mode: 'same-origin',
            credentials: 'same-origin',
            cache: 'no-store',
            redirect: 'follow'
        });
    }
    catch(error) {
        setError('REST API error for ' + method + ' ' + uri);
        return;
    }

    try {
        const data = await response.json();
        if (data.error) {
            setError(data.error);
        }
        else {
            callback(data);
        }
    }
    catch(error) {
        setError('Can not parse response from %{uri}', {"uri": uri});
    }
}

//main
socketConnect();
getGPIOs()

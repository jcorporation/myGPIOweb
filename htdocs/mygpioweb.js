let socket = null;

const modalGPIOinfoInit = BSN.Modal.getInstance(document.getElementById('modalGPIOinfo'));
const modalGPIOsetInit = BSN.Modal.getInstance(document.getElementById('modalGPIOset'));
const modalGPIOblinkInit = BSN.Modal.getInstance(document.getElementById('modalGPIOblink'));

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
    document.getElementById('websocketState').textContent = 'Connecting';
    if (socket != null) {
        socket.onclose = function() {};
        socket.close();
        socket = null;
    }
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
        const obj = JSON.parse(msg.data);
        updateGPIOvalue(obj.gpio);
    }
}

function updateGPIOvalue(gpio) {
    const tr = document.getElementById('gpio' + gpio);
    httpRequest('GET', '/api/gpio/' + gpio, null, function(data) {
        tr.childNodes[2].textContent = data.value;
    });
}

function toggleGPIO(event) {
    const body = JSON.stringify({
        "action": "gpiotoggle"
    });
    const gpio = event.target.closest('tr').data.gpio;
    httpRequest('POST', '/api/gpio/' + gpio, body, function(data) {
        updateGPIOvalue(gpio);
    });
}

function showModalSetGPIO(event) {
    const gpio = event.target.closest('tr').data.gpio;
    document.getElementById('modalGPIOsetGPIO').value = gpio;
    modalGPIOsetInit.show();
}

function setGPIO() {
    const gpio = document.getElementById('modalGPIOsetGPIO').value;
    const valueEl = document.getElementById('modalGPIOsetValue');
    const value = valueEl.options[valueEl.selectedIndex].value;
    const body = JSON.stringify({
        "action": "gpioset",
        "value": value
    });
    httpRequest('POST', '/api/gpio/' + gpio, body, function(data) {
        updateGPIOvalue(gpio);
    });
}

function showModalBlinkGPIO(event) {
    const gpio = event.target.closest('tr').data.gpio;
    document.getElementById('modalGPIOblinkGPIO').value = gpio;
    modalGPIOblinkInit.show();
}

function blinkGPIO() {
    const gpio = document.getElementById('modalGPIOblinkGPIO').value;
    const timeout = Number(document.getElementById('modalGPIOblinkTimeout').value)
    const interval = Number(document.getElementById('modalGPIOblinkInterval').value)
    const body = JSON.stringify({
        "action": "gpioblink",
        "timeout": timeout,
        "interval": interval
    });
    httpRequest('POST', '/api/gpio/' + gpio, body, function(data) {
        updateGPIOvalue(gpio);
    });
}

function infoGPIO(event) {
    const gpioInfoEl = document.getElementById('modalGPIOinfoList');
    gpioInfoEl.textContent = '';
    const gpio = event.target.closest('tr').data.gpio;
    httpRequest('OPTIONS', '/api/gpio/' + gpio, null, function(data) {
        const keys = Object.keys(data.data);
        for (const key of keys) {
            const tr = document.createElement('tr');
            const td1 = document.createElement('td');
            td1.textContent = key;
            tr.appendChild(td1);
            const td2 = document.createElement('td');
            td2.textContent = data.data[key];
            tr.appendChild(td2);
            gpioInfoEl.appendChild(tr);
        }
        modalGPIOinfoInit.show();
    });
}

function createActionLink(icon, title, callback) {
    const a = document.createElement('a');
    a.innerHTML = icon;
    a.href = '#';
    a.title = title;
    a.classList.add('me-2','btn','btn-sm','btn-secondary');
    a.addEventListener('click', function(event) {
        event.preventDefault();
        callback(event);
    }, false);
    return a;
}

function getGPIOactions(direction) {
    const td = document.createElement('td');
    td.appendChild(createActionLink('&#x1F6C8', 'Info', infoGPIO));
    if (direction === 'out') {
        td.appendChild(createActionLink('&#x25e9', 'Toggle', toggleGPIO));
        td.appendChild(createActionLink('&#x2713', 'Set', showModalSetGPIO));
        td.appendChild(createActionLink('&#x2600', 'Blink', showModalBlinkGPIO));
    }
    return td;
}

function getGPIOs() {
    const gpiosEl = document.getElementById('gpios');
    gpiosEl.textContent = '';
    httpRequest('GET', '/api/gpio', null, function(data) {
        for (let i = 0; i < data.entries; i++) {
            const tr = document.createElement('tr');
            for (const k of ['gpio', 'direction', 'value']) {
                const td = document.createElement('td');
                td.textContent = data.data[i][k];
                tr.appendChild(td);
            }
            tr.data = data.data[i];
            tr.appendChild(getGPIOactions(data.data[i].direction));
            tr.setAttribute('id', 'gpio' + data.data[i].gpio);
            gpiosEl.appendChild(tr);
        }
    });
}

async function httpRequest(method, path, body, callback) {
    const uri = getUri('http') + path;
    let response = null;
    try {
        response = await fetch(uri, {
            method: method,
            mode: 'same-origin',
            credentials: 'same-origin',
            cache: 'no-store',
            redirect: 'follow',
            body: body
        });
    }
    catch(error) {
        setError('REST API error for ' + method + ' ' + uri);
        return;
    }
    let data = null;
    try {
        data = await response.json();
    }
    catch(error) {
        setError('Can not parse response from ' + uri);
    }
    if (data.error) {
        setError(data.error);
    }
    else {
        callback(data);
        setError('OK');
    }
}

//main
socketConnect();
getGPIOs()

document.getElementById('websocketReconnect').addEventListener('click', function(event) {
    event.preventDefault();
    socketConnect();
}, false);

document.getElementById('gpioRefresh').addEventListener('click', function(event) {
    event.preventDefault();
    getGPIOs();
}, false);

document.getElementById('modalGPIOblinkSet').addEventListener('click', function(event) {
    blinkGPIO();
    modalGPIOblinkInit.hide();
}, false);

document.getElementById('modalGPIOsetSet').addEventListener('click', function(event) {
    setGPIO();
    modalGPIOsetInit.hide();
}, false);

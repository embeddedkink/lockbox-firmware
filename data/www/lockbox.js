window.onload = Init;

function Init() {
    UpdateAPI();
    SetFormBehaviour("passwordform");
    SetFormBehaviour("settingsform");
    SetFormBehaviour("resetform");
    SetButtonListeners();
    UpdateBoxInfo();
}

function SetButtonListeners() {
    document.getElementById("generateKeyButton").addEventListener("click", function() {GenerateKey('generateKeyLength');});
    document.getElementById("downloadKeyButton").addEventListener("click", function() {DownloadKey();});
    document.getElementById("clearKeyButton").addEventListener("click", function() {ClearKey();});
}

function SetFormBehaviour(formID) {
    var form = document.getElementById(formID);
    function onSubmit(event) {
        if (event) {
            event.preventDefault();
            matches = form.querySelectorAll('button[type="submit"]');
            matches.forEach((match) => {
                match.setAttribute("aria-busy", "true")
            });

            action = event.target.getAttribute("action");
            formaction = document.activeElement.getAttribute("formaction");

            Post(form, (formaction ? formaction : action));
        }
    }
    form.addEventListener('submit', onSubmit, false);
    form.submit = onSubmit;
}

function Post(form, endpoint) {
    var data = new FormData(form);
    var xhr = new XMLHttpRequest();
    var remote = document.getElementById("remoteAPI").value.concat(endpoint);
    xhr.open("POST", remote);
    xhr.timeout = 5000;
    xhr.onload = function () {
        console.log(this.response);
        matches = form.querySelectorAll('button[type="submit"]');
        matches.forEach((match) => {
            match.setAttribute("aria-busy", "false")
        });
        var response = JSON.parse(this.response)
        if (response["result"] != "success")
        {
            form.querySelector('input[name="password"]').setAttribute("aria-invalid", "true");
            if (response["error"] == "BadPassword")
            {
            }
            else if (response["error"] == "NotLocked")
            {
            }
            else
            {
                alert("Box responded with unexpected error: " + response["error"]);
            }
        }
        else
        {
            form.querySelector('input[name="password"]').setAttribute("aria-invalid", "false");
        }
        UpdateBoxInfo();
    };
    xhr.ontimeout = function () {
        console.log(this.response);
        matches = form.querySelectorAll('button[type="submit"]');
        matches.forEach((match) => {
            match.setAttribute("aria-busy", "false")
        });
        DisplayBoxUnreachable();
    }
    xhr.onerror = function () {
        console.log(this.response);
        matches = form.querySelectorAll('button[type="submit"]');
        matches.forEach((match) => {
            match.setAttribute("aria-busy", "false")
        });
        DisplayBoxUnreachable();
    }
    
    xhr.send(data);
}

function DownloadKey() {
    var key = document.getElementById("lockformPassword").value;
    var element = document.createElement('a');
    element.setAttribute('href', 'data:text/plain;charset=utf-8,' + encodeURIComponent(key));
    element.setAttribute('download', "key.txt");
    element.style.display = 'none';
    document.body.appendChild(element);
    element.click();
    document.body.removeChild(element);
}

function GenerateKey(formInput) {
    size = document.getElementById(formInput).value;
    var result = '';
    var characters = 'abcdefghkmnpqrstuvwxy3456789';
    var charactersLength = characters.length;
    for (var i = 0; i < size; i++) {
        result += characters.charAt(Math.floor(Math.random() * charactersLength));
    }
    document.getElementById("lockformPassword").value = result;
}

function ClearKey() {
    passwordinput = document.getElementById("lockformPassword");
    passwordinput.value = "";
    passwordinput.setAttribute("aria-invalid", "");
}

function DisplayBoxUnreachable()
{
    document.getElementById("lockIndicator").innerHTML = "&#10060";
    alert("Box seems to be offline");
}

function UpdateBoxInfo() {
    var xhr = new XMLHttpRequest();
    var remote = document.getElementById("remoteAPI").value.concat("/settings");
    xhr.open("GET", remote);
    xhr.timeout = 5000;
    xhr.onload = function () {
        var response = JSON.parse(this.response)
        document.title = response["data"]["name"];
        document.getElementById("firmwareVersion").innerHTML = response["data"]["version"];
        document.getElementById("settingsformName").value = response["data"]["name"];
        document.getElementById("settingsformOpenPosition").value = response["data"]["servo_open_position"];
        document.getElementById("settingsformClosedPosition").value = response["data"]["servo_closed_position"];
        if (response["data"]["locked"]) {
            document.getElementById("lockIndicator").innerHTML = "&#128274";
        }
        else {
            document.getElementById("lockIndicator").innerHTML = "&#128275";
        }
    };
    xhr.ontimeout = function () {
        DisplayBoxUnreachable();
    }
    xhr.onerror = function () {
        DisplayBoxUnreachable();
    }
    xhr.send();
}
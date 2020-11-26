var getUrl = window.location;
var baseUrl = getUrl.protocol + "//" + getUrl.host + "/" + getUrl.pathname.split('/')[1];
var loading = document.getElementById("loading");
var status = document.getElementById("status");
var busy = document.getElementById("busy");
var input_container = document.getElementById("input_container");
var passwd = document.getElementById("pass_input");

document.addEventListener("DOMContentLoaded", page_ready);

function page_ready()
{
    document.getElementById("scan_button").addEventListener("click", rescan);
    document.getElementById("connect_button").addEventListener("click", connect);

    let nets;
    nets = document.getElementsByClassName("net");
    Array.from(nets).forEach(element => {
        element.addEventListener("click", select_net);
    }); 

}

function rescan()
{
    let xhr = new XMLHttpRequest;
    let formdata = new FormData();
    formdata.append("method", "scan");
    xhr.open('POST', baseUrl, true);
    xhr.send(formdata);
    loading.classList.add("is-v-on", "is-primary");
    loading.innerHTML = "Scanning...";
    input_container.classList.remove("is-v-on");
    passwd.value = '';
    xhr.onreadystatechange = function () 
    {
        if (xhr.readyState == 4) {
            if(this.responseText)
            {
                document.getElementById("scan_list").innerHTML = this.responseText;
                loading.classList.remove("is-v-on");
                let nets;
                nets = document.getElementsByClassName("net");
                Array.from(nets).forEach(element => {
                    element.addEventListener("click", select_net);
                }); 
            }
            else
            {
                busy.classList.add("is-v-on", "is-danger");
                setTimeout(() => busy.classList.remove("is-v-on"), 2000);
                busy.classList.remove("is-v-on");
            }
        }
    }
}

function select_net(e)
{
    let target = e.target;
    let nets = document.getElementsByClassName("net");
    
    Array.from(nets).forEach(element => {
        if(element.innerHTML == target.innerHTML){
            ;
        }
        else
        {
            element.classList.remove("is-success");
            element.removeAttribute("selected");
        }
    }); 
    target.classList.toggle("is-success");
    target.toggleAttribute("selected");
    let ssid = document.querySelector("[selected]");
    if(!ssid)
    {
        input_container.classList.remove("is-v-on");
        passwd.value = '';
    }
    else
    {
        input_container.classList.add("is-v-on");
    }
    
}

function connect()
{
    let xhr = new XMLHttpRequest;
    let formdata = new FormData();
    let ssid = document.querySelector("[selected]");
    let status = document.getElementById("status");

    if(ssid)
    {
        formdata.append("method", "connect");
        formdata.append("ssid", ssid.innerHTML);
        if(passwd.value)
        {
            formdata.append("passwd", passwd.value);
        }
        xhr.open('POST', baseUrl + 'connect', true);
        xhr.timeout = 2000;
        xhr.send(formdata);
        loading.classList.add("is-v-on", "is-primary");
        loading.innerHTML = "Connection...";
        xhr.onreadystatechange = function () 
        {
            if (xhr.readyState == 4) {
                loading.classList.remove("is-v-on");
                if(this.responseText == 'OK')
                {
                    status.classList.add("is-v-on", "is-success");
                    status.innerHTML = "Try to connect to selected AP. If you connected to device via STA mode this page will be no longer operational. Reloading...";
                    setTimeout(() => status.classList.remove("is-v-on", "is-success"), 2000);
                    location.reload();  
                }
                else
                {
                    status.classList.add("is-v-on", "is-danger");
                    status.innerHTML = "You was disconnected. Perhaps you performed wifi connection into different network. At last something terrible could happend :(";
                    setTimeout(() => status.classList.remove("is-v-on", "is-danger"), 4000); 
                    location.reload();  
                }   
            }
        }
    }
    else
    {
        status.classList.add("is-v-on", "is-warning");
        status.innerHTML = "AP doesn't selected";
        setTimeout(() => status.classList.remove("is-v-on", "is-warning"), 2000);
        
    }
}



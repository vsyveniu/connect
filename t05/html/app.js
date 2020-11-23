var getUrl = window.location;
var baseUrl = getUrl.protocol + "//" + getUrl.host + "/" + getUrl.pathname.split('/')[1];
var loading = document.getElementById("loading");
var busy = document.getElementById("busy");

document.addEventListener("DOMContentLoaded", page_ready);

function page_ready()
{
    document.getElementById("scan_button").addEventListener("click", rescan);
}


function rescan()
{
    let xhr = new XMLHttpRequest;
    let formdata = new FormData();
    formdata.append("method", "scan");
    xhr.open('POST', baseUrl, true);
    xhr.send(formdata);
    loading.classList.add("is-visible");
    xhr.onreadystatechange = function () 
    {
        if (xhr.readyState == 4) {
            if(this.responseText)
            {
                document.getElementById("scan_list").innerHTML = this.responseText;
                loading.classList.remove("is-visible");
            }
            else
            {
                busy.classList.add("is-visible");
                setTimeout(2000);
                busy.classList.remove("is-visible");
            }
        }
    }
}


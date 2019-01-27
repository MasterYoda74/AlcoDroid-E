var jsonConfig;
var jsonUsers;
var jsonShots;

function CreateRequest()
{
    var Request = false;

    if (window.XMLHttpRequest)
    {
        //Gecko-совместимые браузеры, Safari, Konqueror
        Request = new XMLHttpRequest();
    }
    else if (window.ActiveXObject)
    {
        //Internet explorer
        try
        {
             Request = new ActiveXObject("Microsoft.XMLHTTP");
        }    
        catch (CatchException)
        {
             Request = new ActiveXObject("Msxml2.XMLHTTP");
        }
    }
    if (!Request)
    {
        alert("Невозможно создать XMLHttpRequest");
    }
    return Request;
} 

/*
Функция посылки запроса к файлу на сервере
r_method  - тип запроса: GET или POST
r_path    - путь к файлу
r_args    - аргументы вида a=1&b=2&c=3...
r_handler - функция-обработчик ответа от сервера
*/
function SendRequest(r_method, r_path, r_args, r_handler)
{
    //Создаём запрос
    var Request = CreateRequest();
    
    //Проверяем существование запроса еще раз
    if (!Request)
    {
        return;
    }
    
    //Назначаем пользовательский обработчик
    Request.onreadystatechange = function()
    {
        //Если обмен данными завершен
        if (Request.readyState == 4)
        {
            //Передаем управление обработчику пользователя
            if (IsJsonString(Request.responseText)) {
                r_handler(JSON.parse(Request.responseText));
            } else {
                r_handler(Request.responseText);
            }
        }
    }
    
    //Проверяем, если требуется сделать GET-запрос
    if (r_method.toLowerCase() == "get" && r_args.length > 0)
    r_path += "?" + r_args;
    
    //Инициализируем соединение
    Request.open(r_method, r_path, true);
    
    if (r_method.toLowerCase() == "post")
    {
        //Если это POST-запрос
        
        //Устанавливаем заголовок
        Request.setRequestHeader("Content-Type","application/x-www-form-urlencoded; charset=utf-8");
        //Посылаем запрос
        Request.send(r_args);
    }
    else
    {
        //Если это GET-запрос
        
        //Посылаем нуль-запрос
        Request.send(null);
    }
} 

function IsJsonString(str) {
    try {
        JSON.parse(str);
    } catch (e) {
        return false;
    }
    return true;
}

function parseconfig(configJ) {
    //alert(jsonobj.responseText);
    //var configJ = JSON.parse(jsonobj.responseText);
    jsonConfig = configJ;
    run_socket();
    if (configJ.isAP) {
        document.getElementById("isAP").setAttribute("checked","checked");
    } else {
        document.getElementById("isAP").removeAttribute("checked");
    }    
    document.getElementById("APSSID").value=configJ.APSSID;
    document.getElementById("APPASS").value=configJ.APPASS;
    document.getElementById("wifiSSID").value=configJ.wifiSSID;
    document.getElementById("wifiPASS").value=configJ.wifiPASS;
    document.getElementById("SMAX").value=configJ.servoMAX;
    document.getElementById("SMIN").value=configJ.servoMIN;
    document.getElementById("Steps").value=configJ.stepPerMl;
    document.getElementById("Start").value=configJ.startMl;
    document.getElementById("Feedback").value=configJ.feedback;
    document.getElementById("LED").value=configJ.ledbright;
}
function parseshots(configJ) {
    //alert(jsonobj.responseText);
    //var configJ = JSON.parse(jsonobj.responseText);
    jsonShots=configJ;
    //alert(configJ.shots.length);
    var parent=document.getElementById("shots");
    parent.innerHTML="";
    for (i=0; i<configJ.shots.length; i++) {
        var newline=document.createElement('div');
        newline.className="container-fluid";
        newline.id="shot"+configJ.shots[i].ID;
        var div1=document.createElement('div');
        div1.className="col-xs-6";
        var div2=document.createElement('div');
        div2.className="col-xs-4";
        var div3=document.createElement('div');
        div3.className="col-xs-2";        
        var newinput=document.createElement('div');
        newinput.className="form-control";
        //newinput.style="width: 40%;";
        //newinput.pattern="[0-9a-zA-Zа-яА-Я.\- ]{1,20}";
        newinput.innerText=configJ.shots[i].ID;
        div1.appendChild(newinput);
        newline.appendChild(div1);
        var newdoze=document.createElement('div');
        newdoze.className="form-control";
        newdoze.style="background-color: "+configJ.shots[i].color+";";
        div2.appendChild(newdoze);
        newline.appendChild(div2);
        var newbutt=document.createElement('div');
        newbutt.className="btn btn-block btn-success btn-plus";
        //newbutt.style="width: 25%;";
        newbutt.innerText="+";
        newbutt.setAttribute("onclick","shotClick("+i+");");
        div3.appendChild(newbutt);
        newline.appendChild(div3);
        parent.appendChild(newline);
    }
}
function parseusers(configJ) {
    //alert(jsonobj.responseText);
    //var configJ = JSON.parse(jsonobj.responseText);
    jsonUsers=configJ;
    var isNew = false;
    //alert(configJ.users.length);
    var parent=document.getElementById("users");
    parent.innerHTML="";
    for (i=0; i<configJ.users.length; i++) {
        if (configJ.users[i].ID) {
        var newline=document.createElement('div');
        newline.className="container-fluid no-gutters";
        //newline.id=
        
        var div1=document.createElement('div');
        div1.className="col-xs-12 id-num";
        div1.innerHTML="ID: <span>"+configJ.users[i].ID+"</span>";
        var div2=document.createElement('div');
        div2.className="col-xs-12 col-sm-4";
        var inpname=document.createElement('input');
        inpname.className="form-control";
        inpname.placeholder="Имя";
        inpname.value=configJ.users[i].name;
        inpname.id="userName"+configJ.users[i].ID;
        inpname.setAttribute('oninput','userChange();');
        ///inpname.setAttribute('onblur','userBlur('+i+');')
        div2.appendChild(inpname);
        var div3=document.createElement('div');
        div3.className="col-xs-5 col-sm-3"; 
        var newcolor=document.createElement('div');
        newcolor.className="form-control";
        newcolor.style="background-color: "+configJ.users[i].color+";";
        div3.appendChild(newcolor);
        var div4=document.createElement('div');
        div4.className="col-xs-5 col-sm-3";
        var newdoze=document.createElement('input');
        newdoze.type="number";
        newdoze.className="form-control";
        newdoze.placeholder="Доза";
        newdoze.min="5";
        newdoze.max="50";
        newdoze.step="5";
        newdoze.id="userDoze"+configJ.users[i].ID;
        newdoze.value=configJ.users[i].doze;
        newdoze.setAttribute('oninput','userChange();');
        ///newdoze.setAttribute('onblur','userBlurDoze('+i+');')
        div4.appendChild(newdoze);
        var div5=document.createElement('div');
        div5.className="col-xs-2 col-sm-2";
        var newbutt=document.createElement('div');
        newbutt.className="btn btn-block btn-danger btn-minus";
        newbutt.innerText="-";
        newbutt.id="userMinus_"+i;
        //newbutt.onclick== function() { userClick(i) };
        newbutt.setAttribute("onclick","userClick("+i+");");
        div5.appendChild(newbutt);
        newline.appendChild(div1);
        newline.appendChild(div2);
        newline.appendChild(div3);
        newline.appendChild(div4);
        newline.appendChild(div5);
        parent.appendChild(newline);
        var corr=document.getElementById("shot"+configJ.users[i].ID);
        if (corr) corr.lastChild.innerText="";
        if (configJ.users[i].color == "grey" && !isNew) {
            document.getElementById("addShot").style ="display: block;";
            document.getElementById("new-shot").value = configJ.users[i].ID;
            document.getElementById("new-color").value = "";
            isNew=true;
        }
        }
    }
    fillstat(configJ);
}
function fillstat(j) {
    var tml=0;
    var tshots=0;
    var parent=document.getElementById('status')
    parent.innerHTML="";
    for (i=0; i<j.users.length; i++) {
        var newline=document.createElement('div');
        newline.className="container-fluid list";
        newline.id="stat"+j.users[i].ID;
        var div1=document.createElement('div');
        div1.className="col-sm-6 col-xs-6";
        div1.innerText=j.users[i].name;
        newline.appendChild(div1);
        div1=document.createElement('div');
        div1.className="col-sm-3 col-xs-3";
        div1.innerText=j.users[i].totalshots;
        newline.appendChild(div1);
        div1=document.createElement('div');
        div1.className="col-sm-3 col-xs-3";
        div1.innerText=j.users[i].total;
        newline.appendChild(div1);
        parent.appendChild(newline);
        tml=tml+j.users[i].total;
        tshots=tshots+j.users[i].totalshots;
    }
    document.getElementById('tml').innerText  =tml+" ml";
    document.getElementById("tshots").innerText=tshots;
}

var userButPressed;
var timerID;
function userClick(index) {
    if (userButPressed==index) {
        clearTimeout(timerID);
        jsonUsers.users.splice(index,1);
        parseshots(jsonShots);
        parseusers(jsonUsers);
        userSave();
        userButPressed=undefined;
    } else {
        clearTimeout(timerID);
        userButPressed = index;
        for(i=0;i<jsonUsers.users.length;i++) {
            document.getElementById("userMinus_"+i).innerText = "-";
        }
        document.getElementById("userMinus_"+index).innerText = "Точно?";
        timerID=setTimeout(function(){
            document.getElementById("userMinus_"+index).innerText = "-";
            userButPressed=undefined;
        }, 3000);
    }
}



function shotClick(index) {
    for (i=0; i<jsonUsers.users.length;i++) {
        if (jsonUsers.users[i].ID == jsonShots.shots[index].ID) {
            alert("Shot is used!")
            return;
        }
    }
    jsonUsers.users.push({
        'ID': jsonShots.shots[index].ID, 
        'name': '',
        'color': jsonShots.shots[index].color,
        'doze': 0,
        'total': 0,
        'totalshots': 0        
    });
    parseusers(jsonUsers);
}

function userSave() {
    //alert("User save");
    //alert(JSON.stringify(jsonUsers));
    for (i=0;i<jsonUsers.users.length;i++){
        jsonUsers.users[i].name=document.getElementById("userName"+jsonUsers.users[i].ID).value;
        jsonUsers.users[i].doze=document.getElementById("userDoze"+jsonUsers.users[i].ID).value;
    }
    SendRequest("GET", "saveusers","json="+JSON.stringify(jsonUsers), function(inp){
        if (inp == "OK") {
            fillstat(jsonUsers);
            document.getElementById("userSave").style ="display: none;";
        }
    });

}
function userChange(){
    document.getElementById("userSave").style ="display: inline-block;";
}

function addShot() {
    //alert("Add shot");
    var shotID = document.getElementById("new-shot").value;
    var shotColor= document.getElementById("new-color").value;
    jsonShots.shots.push({
        'ID': shotID, 
        'color': shotColor       
    });
    SendRequest("GET", "saveshot","json="+JSON.stringify(jsonShots), function(inp){
        if (inp == "OK") {
            document.getElementById("addShot").style ="display: none;";
        }
    });
    for (i=0; i<jsonUsers.users.length;i++) {
        if (jsonUsers.users[i].ID == shotID) {
            jsonUsers.users[i].color = shotColor;
        }
    }
    document.getElementById("addShot").style ="display: none;";
    parseshots(jsonShots);
    parseusers(jsonUsers);
    userSave();
}

function saveconfig() {
    jsonConfig.APSSID=document.getElementById("APSSID").value;
    jsonConfig.APPASS=document.getElementById("APPASS").value;
    jsonConfig.wifiSSID =document.getElementById("wifiSSID").value;
    jsonConfig.wifiPASS =document.getElementById("wifiPASS").value;
    jsonConfig.servoMAX =document.getElementById("SMAX").value;
    jsonConfig.servoMIN =document.getElementById("SMIN").value;
    jsonConfig.stepPerMl =document.getElementById("Steps").value;
    jsonConfig.startMl =document.getElementById("Start").value;
    jsonConfig.feedback =document.getElementById("Feedback").value;
    jsonConfig.ledbright =document.getElementById("LED").value;
    jsonConfig.isAP =document.getElementById("isAP").checked;
    SendRequest("GET", "saveconfig","json="+JSON.stringify(jsonConfig), function(inp){
        if (inp == "OK") {
            document.getElementById("SaveConfig").value ="OK";
        }
    });
}

function startSystem() {
    SendRequest("GET", "start", "", function(inp){
        if (inp == "OK") {
            document.getElementById("startSys").style = "display: none;";
        }
    });
}

function dataProc(jdata){
    if (jdata.dock) {
        var doc = document.getElementById("dock-"+(jdata.dock[0].pos+1));
        switch (jdata.dock[0].state){
            case -1:
            doc.className = "col-xs-4 col-sm-2 dock";
            doc.innerText=jdata.dock[0].pos+1;
            parseshots(jsonShots);
            parseusers(jsonUsers);
            break;
            case 0:
            doc.className = "col-xs-4 col-sm-2 dock dock-empty";
            doc.innerText=jsonUsers.users[jdata.dock[0].user].name;
            break;
            case 1:
            doc.className = "col-xs-4 col-sm-2 dock dock-full";
            doc.innerText=jsonUsers.users[jdata.dock[0].user].name;
            SendRequest("GET","users.json","",parseusers);
            break;
            case 2:
            doc.className = "col-xs-4 col-sm-2 dock dock-new";
            SendRequest("GET","shots.json","",parseshots);
            SendRequest("GET","users.json","",parseusers);
            break;
            default:
            break;
        }
    }
    if (jdata.userreload) {
        SendRequest("GET","shots.json","",parseshots);
        SendRequest("GET","users.json","",parseusers);
    }
}
function run_socket() {
    //var connection = new WebSocket(url.replace('socket ','ws://'), ['arduino']);
    var connection = new WebSocket('ws://'+jsonConfig.ip+":81", ['arduino']);
    connection.onopen = function () {
     connection.send('Connect ' + new Date());
    };
    connection.onerror = function (error) {
     console.log('WebSocket Error ', error);
    };
    connection.onclose = function (e) {
        console.log('WebSocket Close ', e);
    }
    connection.onmessage = function (e) {
     console.log('Server: ', e.data);
     dataProc(JSON.parse(e.data));
     //jsonResponse_news = mergeObject(jsonResponse, socket_data);

    }
   }



//  MAIN CODE
SendRequest("GET","config.json","",parseconfig);
SendRequest("GET","shots.json","",parseshots);
SendRequest("GET","users.json","",parseusers);


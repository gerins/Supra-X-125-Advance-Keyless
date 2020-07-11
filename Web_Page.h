char homePage[] PROGMEM = R"=====(
<html>
<head>
</head>
<body>
<p> LED State: <span id="led-state">__</span> </p> 
<button onclick = "myFunction()" > TOGGLE LED ON OFF </button>
<h1>ESP8266 Web Server</h1>
<script>
function myFunction()
{
  console.log("button Clicked!");
  var xhr = new XMLHttpRequest();
  var url = "/relaystate";

  xhr.onreadystatechange = function()
  {
    if(this.readyState == 4 && this.status == 200)
    {
     document.getElementById("led-state").innerHTML = this.responseText;   
    }
  };
  xhr.open("GET", url, true);
  xhr.send(); 
}
</script>
</body>
</html>
)=====";

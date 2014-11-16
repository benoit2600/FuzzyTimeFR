
var mConfig = {};

Pebble.addEventListener("ready", function(e) {
       // console.log("fuzzy v2.0 is ready");
  loadLocalData();
  returnConfigToPebble();
});

Pebble.addEventListener("showConfiguration", function(e) {
		//console.log("ouverture URL");
        Pebble.openURL(mConfig.configureUrl);
	
});

Pebble.addEventListener("webviewclosed",
  function(e) {
    if (e.response) {
      var config = JSON.parse(e.response);
      saveLocalData(config);
      returnConfigToPebble();
    }
  }
);

function saveLocalData(config) {

  //console.log("saveLocalData() " + JSON.stringify(config));

  localStorage.setItem("CminutePrecise", parseInt(config.CminutePrecise));  
  localStorage.setItem("Ccouleur", parseInt(config.Ccouleur)); 
  localStorage.setItem("Cbatterie", parseInt(config.Cbatterie)); 
  localStorage.setItem("Ctopbar", parseInt(config.Ctopbar)); 
  
  loadLocalData();

}
function loadLocalData() {
  
  mConfig.CminutePrecise = parseInt(localStorage.getItem("CminutePrecise"));
  mConfig.Ccouleur = parseInt(localStorage.getItem("Ccouleur"));
  mConfig.Cbatterie = parseInt(localStorage.getItem("Cbatterie"));
  mConfig.Ctopbar = parseInt(localStorage.getItem("Ctopbar"));
  mConfig.configureUrl = "http://benoit2600.url.ph/index.html";

  //console.log("loadLocalData() " + JSON.stringify(mConfig));
}
function returnConfigToPebble() {
  //console.log("Configuration window returned: " + JSON.stringify(mConfig));
  Pebble.sendAppMessage({
    "CminutePrecise":parseInt(mConfig.CminutePrecise), 
    "Ccouleur":parseInt(mConfig.Ccouleur), 
    "Cbatterie":parseInt(mConfig.Cbatterie), 
    "Ctopbar":parseInt(mConfig.Ctopbar)
  });    
}

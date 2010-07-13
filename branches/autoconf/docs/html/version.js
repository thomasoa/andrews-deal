currentDealVersion = "<?php echo $version;?>";

function noteOldVersion(version) {
  if (version != currentDealVersion) {
     element = document.getElementById("versionWarning");
     if (element != null) {
       element.innerHTML = "This is not the current version of Deal.  " +
                           "The current version of Deal is " + currentDealVersion + 
                           " which can be found at " + 
                           "<a href='<?php echo $bridgeFantasia;?>deal/'>the main Deal site.</a>";
       element.style.display = "block";
     }
  }
}

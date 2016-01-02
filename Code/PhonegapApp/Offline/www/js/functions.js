    
  function emptyFunction() {
        // do something
    }
	
	// Show a custom alert
		
		function showAlert(title, callback, message, button) {
			navigator.notification.alert(
				message,          // title
				callback,         // callback
				title,            // message
				button            // buttonName
			);
		}

		function showConfirm (title, callback, message, buttons){
			navigator.notification.confirm(
			message,           // message
			callback,          // callback to invoke with index of button pressed
			title,             // title
			buttons            // buttonLabels
			);

		}
		
		
		function refresh () {
			/* if (!checkConnection()){
					alert ("No internet connection detected. Connect to internet and try again to download list.");
		    } else { */
				if ( $.jStorage.get("drinkDatabase_storage") == null || $.jStorage.get("drinkDatabase_storage") == ""){
					alert ("Insert address of the database under settings section");			 
				 } else {
					alert("loading list");
					//window.plugins.toast.showShortCenter('Loading List');
					model.setJson();
				}	
			//}
		}
		
		
		function checkConnection() {
			var networkState = navigator.connection.type;

			var states = {};
			states[Connection.UNKNOWN]  = 'Unknown connection';
			states[Connection.ETHERNET] = 'Ethernet connection';
			states[Connection.WIFI]     = 'WiFi connection';
			states[Connection.CELL_2G]  = 'Cell 2G connection';
			states[Connection.CELL_3G]  = 'Cell 3G connection';
			states[Connection.CELL_4G]  = 'Cell 4G connection';
			states[Connection.CELL]     = 'Cell generic connection';
			states[Connection.NONE]     = 'No network connection';

			if(networkState == "none"){
				return false;	
			} else {
				return true;
			}
}

		function extractAmount (amount){
			if (amount.charAt(amount.length -2) == ","){
				return "0"+amount.charAt(amount.length -1);
			} else {
				return amount.charAt(amount.length -2) + amount.charAt(amount.length -1);
			}
		}
		
		
		


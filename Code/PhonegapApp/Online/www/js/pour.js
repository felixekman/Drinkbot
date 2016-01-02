var pour = (function(module) {

	var args;

	module.drink = function (drinkName, c0, c1, c2, c3, c4, c5, c6, c7){
		args = "P0:"+c1+";P1:"+c4+";P2:"+c3+";P3:"+c2+";P4:"+c5+";P5:"+c7+";P6:"+c6+";P7:"+c0;
		var title = "New Order";
		var message = "Pour "+ drinkName + "?";
		callback;
		var buttons =  ['Pour','Cancel'];
		showConfirm (title, callback, message, buttons);
	}
	
	function callback (index) {
		if (index == 1){
			pourFunction();
		} else {
			alert ("Order cancelled.");
		}
	}
	
	function pourFunction(){
	
		var unitId = $.jStorage.get("unitID_storage");
        var accessToken = $.jStorage.get("accessToken_storage");
        //This builds the URL to the REST API endpoint for the setpumps function
        //with your given coreId
        var url = "https://api.spark.io/v1/devices/" + unitId + "/setpumps";
		alert(args);
        //Turn on the alertInfo div to show the user that the pumping is being attempted
        //Make the Ajax Call
        $.ajax({
          type: "POST",
          url: url,
          data: {
             access_token: accessToken,
             args: args
          },
		  success: function (data, textStatus, jqXHR){
			if(data.return_value == 0){
				window.plugins.toast.showShortCenter("Order completed");
			} else {
				window.plugins.toast.showShortCenter("Error completing order");
			}
		  },
          complete: function (jqxhr, status) {
             //Figure out if the call was successful or not
             //The setpumps function should return a value of 0 if all was well
             //If we got anything else back, it failed. Use that knowledge to show
             //the appropriate alert div.
          }
        });
     };
	
	return module;
}(	pour || {}));


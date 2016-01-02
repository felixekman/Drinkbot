	function settingsFormf() {
	
		var ok = true;
		$("#settingsForm label").removeClass("invalid");
		$("#settingsForm input").each (function (){
			if($(this).val() == ""){
				var label = $('label[for="'+$(this).attr('id')+'"]');
				label.addClass("invalid");
				ok = false;
			}else {
				$.jStorage.set($(this).attr('id') + "_storage", $(this).val() )
			}
		});
			
			if (ok){
				showAlert("Submitted", emptyFunction, "Thank you.", 'OK');
				$.mobile.changePage( "#settingsSection", { allowSamePageTransition: true } );
				return false;
			} else {
				showAlert("Not submitted", emptyFunction, "Please fill in all fields.", 'OK');
				$.mobile.changePage( "#settingsSection", { allowSamePageTransition: true } );
				return false;			
			}
	}

	function deleteSettings() {
			var r = confirm("Are you sure?");
			if (r == true) {
				$.jStorage.set("unitID_storage",  "");
				$.jStorage.set("accessToken_storage", "");
				$.jStorage.set("drinkDatabase_storage", "");
				$.mobile.changePage( "#settingsSection", { allowSamePageTransition: true } );
			} else {
				alert("Cancelled");
			}
		return false;
	} 
	
	
	

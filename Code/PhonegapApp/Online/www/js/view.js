

// ======================================= HOME ======================================

$(document).on("pagebeforeshow", "#home", function (){
	if($.jStorage.get("data")){
			var list;
			var grid = true;
			var json = JSON.parse(model.getJson());
			var list = '<div class="ui-grid-a">';
			for (i = 0; i < json.length; i++){
				if (grid) { 
					var block = '<div class="ui-block-a">';
				} else {
					var block = '<div class="ui-block-b">';
				}
				list = list + block + '<img src="' + json[i].Url + '" id ="' + json[i].Name + '" onclick ="pour.drink(\'' + json[i].Name + '\',\'' +extractAmount(json[i].Content0)+ '\',\'' +extractAmount(json[i].Content1)+ '\',\'' +extractAmount(json[i].Content2)+ '\',\'' +extractAmount(json[i].Content3)+ '\',\'' +extractAmount(json[i].Content4)+ '\',\'' +extractAmount(json[i].Content5)+ '\',\'' +extractAmount(json[i].Content6)+ '\',\'' +extractAmount(json[i].Content7)+ '\' )" /> </div>'; 
				if (grid) {
					grid = false;
				} else {
					grid = true;
				}
			}
			list = list + '</div>';
			$("#homeScreenList").html(list);
			$("#home").trigger("create"); 
	}
});

// ======================================= SETTINGS ======================================


$(document).on("pagebeforeshow", "#settingsSection", function (){
	if ($.jStorage.get("unitID_storage") != null){
		document.settingsForm.unitID.value = $.jStorage.get("unitID_storage");
	}
	if ($.jStorage.get("accessToken_storage") != null){
		document.settingsForm.accessToken.value = $.jStorage.get("accessToken_storage");	
	}
	if ($.jStorage.get("drinkDatabase_storage") != null){
		document.settingsForm.drinkDatabase.value = $.jStorage.get("drinkDatabase_storage");	
	}

	$("#settingsSection").trigger("create");
	

});


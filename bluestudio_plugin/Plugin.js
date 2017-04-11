/*
	genera il file .gatt
*/

function GetInfo(infoObject) 
{
    infoObject.Name = "btstack";
    infoObject.Description = "Genera il file .gatt";
    infoObject.Author = "max";
    infoObject.Version = "0.0";
    infoObject.IsClient = false;
}


function RunPlugin(profiledata)
{
    log("btstack");
	
	var template = FileManager.ReadFile("template.txt");
	var output = ProcessTemplate(template, profiledata);
	FileManager.CreateFile("profilo.gatt",  output);
}

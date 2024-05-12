











'STR#' 32000 "Add-on Name and Description" {
		"Example AddOn"
		"Example AddOn Description..."
}

'STR#' 32500 "Strings for the Menu" {
		"Example AddOn Command"
		"Test ^E3 ^ES ^EE ^EI ^ED ^ET"
		"Convert GDL to 2D-HLM ^E3 ^ES ^EE ^EI ^ED ^ET"
		"Convert 2D-HLM to 3D-HLM ^E3 ^ES ^EE ^EI ^ED ^ET"
		"Check All HLM Floors ^E3 ^ES ^EE ^EI ^ED ^ET"
		"Delete All 2D HLM ^E3 ^ES ^EE ^EI ^ED ^ET"
}

'GDLG' 32600 Modal | grow   40   40  345  128  "Example Dialog" {
 Button				  245   95   90   23	LargePlain  "OK"
 Button				  145   95   90   23	LargePlain  "Cancel"
 Separator			   10   83  325    2
}

'DLGH' 32600 DLG_Example {
1	""		Button_0
2	""		Button_1
3	""		Separator_0
}

import bb.cascades 1.2
import bb.data 1.0
import Network.ListFavoriteController 1.0
import conf.settings 1.0


NavigationPane {
    id: nav
    property int     navDepth
    property variant tpage
    property variant tColPage
    
    property variant itemToTag
    property int     chosenTag
    
    Page {
        
	    Container {
            	        
            ActivityIndicator {
                id: activityIndicator
                horizontalAlignment: HorizontalAlignment.Center
                verticalAlignment: VerticalAlignment.Center
                preferredHeight: 60
            }
	        
	        ListView {
                // ------------------------------------------------------------------
                // Pull to refresh
                signal refreshTriggered()
                property bool loading: false
                id: listFav
                leadingVisualSnapThreshold: 2.0
                leadingVisual: RefreshHeader {
                    id: refreshHandler
                    onRefreshTriggered: {
                        listFav.refreshTriggered();
                    }
                }
                onTouch: {
                    refreshHandler.onListViewTouch(event);
                }
                onLoadingChanged: {
                    refreshHandler.refreshing = refreshableList.loading;
                    
                    if(!refreshHandler.refreshing) {
                        // If the refresh is done 
                        // Force scroll to top to ensure that all items are visible
                        scrollToPosition(ScrollPosition.Beginning, ScrollAnimation.None);
                    }
                }
                
                // ---------------------------------------------------------------------
                // view

	            objectName: "listFav"
	            
	            dataModel: GroupDataModel {
	                id: theModel
	                sortingKeys: ["category"]
	            }
	            
	            listItemComponents: [
	                ListItemComponent {
	                    type: "header"
	                    Header {
	                        title: ListItemData
	                    }
	                },
	                ListItemComponent {
	                    type: "item"
	                    
	                    Container {
                            function colorIndexToHex(index) {
                                switch (index) {
                                    case 0:
                                        return "#ffffff";
                                    
                                    case 1:
                                        return "#a8cc0a";
                                    
                                    case 2:
                                        return "#27bbc6";
                                    
                                    case 3:
                                        return "#fe5e05";
                                    
                                    case 4:
                                        return "#ef0061";
                                    
                                    case 5:
                                        return "#795ae0";
                                    
                                    default:
                                        return "#ffffff";
                                }
                            }
                            
	                        id: titleContainer
                            layout: StackLayout {
                                orientation: LayoutOrientation.LeftToRight
                            }
                            horizontalAlignment: HorizontalAlignment.Fill
                            verticalAlignment: VerticalAlignment.Fill
                            Container {
                                id: tagContainer
                                minWidth: 8
                                maxWidth: 8
                                verticalAlignment: VerticalAlignment.Fill
                                background: Color.create(colorIndexToHex(ListItemData.color))
                            }
                            Container {
                                minWidth: 2
                                maxWidth: 2
                                verticalAlignment: VerticalAlignment.Fill
                            }
	                        
    	                    Container {
                                id: topicContainer
                                
    	                        layout: StackLayout {
    	                            orientation: LayoutOrientation.TopToBottom
    	                        }
    	                        verticalAlignment: VerticalAlignment.Top
    	                        Label {
    	                            text: ListItemData.title
    	                        }
    	                        
    	                        Container {
    	                            layout: DockLayout {
    	                            }
    	                            horizontalAlignment: HorizontalAlignment.Fill
    	
    	                            Label {
    	                                text: ListItemData.lastAuthor + " - " + ListItemData.timestamp
    	                                horizontalAlignment: HorizontalAlignment.Right
    	                                textStyle {
    	                                    base: SystemDefaults.TextStyles.SmallText
                                            color: Application.themeSupport.theme.colorTheme.style == VisualStyle.Dark ? Color.Cyan : Color.Blue
    	                                }
    	                            }
    	                            
    	                            Label {
    	                                text: ListItemData.pages
    	                                horizontalAlignment: HorizontalAlignment.Left
    	                                textStyle {
    	                                    base: SystemDefaults.TextStyles.SmallText
    	                                    color: Color.Gray
    	                                }
    	                            }
    	                        }
    	                        Divider {}
                            }

	                        
                            contextActions: [
                                ActionSet {
                                    title: qsTr("Navigation")
                                    
                                    ActionItem {
                                        title: qsTr("First page")
                                        imageSource: "asset:///images/icon_prev_all.png"
                                        onTriggered: {
                                            titleContainer.ListItem.view.gotoPage(ListItemData.urlFirstPage, ListItemData.title)
                                        }
                                    }
                                    ActionItem {
                                        title: qsTr("Last page")
                                        imageSource: "asset:///images/icon_next_all.png"
                                        onTriggered: {
                                            titleContainer.ListItem.view.gotoPage(ListItemData.urlLastPage, ListItemData.title)
                                        }
                                    }
                                    ActionItem {
                                        title: qsTr("Last page read")
                                        imageSource: "asset:///images/icon_next.png"
                                        onTriggered: {
                                            titleContainer.ListItem.view.gotoPage(ListItemData.urlLastPostRead, ListItemData.title)
                                        }
                                    }
                                    
                                    ActionItem {
                                        title: qsTr("Color Tag")
                                        imageSource: "asset:///images/chickened.png"
                                        onTriggered: {
                                            titleContainer.ListItem.view.openColorPicker(ListItemData.urlFirstPage, titleContainer.ListItem.indexPath)
                                           
                                        }
                                    }
                                    
                                    DeleteActionItem {
                                        title: qsTr("Remove flag")
                                        onTriggered: {
                                            titleContainer.ListItem.view.deleteFlag(ListItemData.urlFirstPage, titleContainer.ListItem.indexPath)
                                        }
                                    }
                                }
                            ]
	                    }
	                }
	            ]
	            
                function gotoPage(urlFirstPage, titleTopic) {
                    if(!tpage) {
                        tpage = threadPage.createObject();
                    }
                    
                    // Set the url of the page to load and thread caption. 
                    tpage.urlPage = urlFirstPage
                    tpage.caption   = titleTopic
                    
                    nav.push(tpage);
                }
                
                function deleteFlag(urlFirstPage, indexPath) {
                    listFavoriteController.deleteFlag(urlFirstPage);
                    listFav.dataModel.removeAt(indexPath);
                }
                
                function openColorPicker(urlFirstPage, indexPath) {
                    if(!tColPage) {
                        tColPage = colorPickerPage.createObject();
                    }
                    
                    // Set the url of the page to load and thread caption. 
                    itemToTag = indexPath;
                    tColPage.urlPage = urlFirstPage
                    nav.push(tColPage);
                    
                }
	            
	            onTriggered: {
	                var chosenItem = dataModel.data(indexPath);
	                
	                // Create the content page and push it on top to drill down to it.
                    if(!tpage) {
                        tpage = threadPage.createObject();
                    }
	                
	                // Set the url of the page to load and thread caption. 
                    tpage.urlPage = chosenItem.urlLastPostRead
                    tpage.caption   = chosenItem.title
	                	                
                    nav.push(tpage);
	            }
	            
	            onRefreshTriggered: {
                    activityIndicator.start();
                    listFavoriteController.getFavorite();
                }
	            
	         }
	        
	         attachedObjects: [
	             ListFavoriteController {
	                 id: listFavoriteController
	                 
	                 onComplete: {
	                        activityIndicator.stop();
	                 }
	             }, 
	             ComponentDefinition {
	                 id: threadPage
	                 source: "ThreadPage.qml"
	             },
                 AppSettings {
                     id: appSettings
                 },
                 ComponentDefinition {
                     id: colorPickerPage
                     source: "ColorPicker.qml"
                 }
	         ]
	         
	    }
	    
	    onCreationCompleted: {
	        
            listFavoriteController.setListView(listFav);
            listFavoriteController.getFavorite();
            activityIndicator.start();
            navDepth = 0;
            chosenTag = -1;
        }
	}
    
    onPopTransitionEnded: {
        --navDepth;
        
        if(navDepth == 1) {
            if(appSettings.autoRefresh) {
            	listFavoriteController.getFavorite();
                activityIndicator.start();
            }
            
            if(chosenTag != -1) {
                var selectedItem = listFav.dataModel.data(itemToTag);
                selectedItem.color = chosenTag;
            }
            chosenTag = -1;
        }
    }
    
    onPushTransitionEnded: {
        ++navDepth;
    }
}

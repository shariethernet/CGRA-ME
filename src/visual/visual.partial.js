function draw_helper(context) {
    var container = document.getElementById('context_' + context.toString());
    var data = {
        nodes: nodes[context],
        edges: edges[context]
    };

    var options = {
        nodes: {
            color: clusterColor[clusterColor.length - 1]
        },
        edges: {
            color: '#848484'
        },
        layout: {
            randomSeed: 0,
            improvedLayout: false
        },
        physics: {
            forceAtlas2Based: {
                gravitationalConstant: -26,
                centralGravity: 0.005,
                springLength: 230,
                springConstant: 0.18
            },
            maxVelocity: 146,
            solver: 'forceAtlas2Based',
            timestep: 0.35,
            stabilization: {iterations: 150}
        }
    };

    // create a network
    var network = new vis.Network(
        container,
        data,
        options
    );

    applyMapping(nodeMapped[context]);
    clusterByModule(clusterTypes[context]);

    var openedCluster = [];
    var firstTime = true;

    // Fit after draw
    network.on("afterDrawing", function (params) {
        if (firstTime) {
            network.fit();
            firstTime = false;
        }
    });

    // if we left double-click on a node, we want to open it
    network.on("doubleClick", function (params) {
        if (params.nodes.length == 1) {
            if (network.isCluster(params.nodes[0]) == true) {
                network.openCluster(params.nodes[0])
                openedCluster.push(params.nodes[0]);
                //network.stabilize();
            }
        }
    });

    // if we right click on a node, we want to close it
    network.on("oncontext", function (params) {
        params.event.preventDefault();
        if (params.nodes.length == 1) {
            var index;
            var parentStr;
            var clusterToClose = [];
            if ((index = openedCluster.lastIndexOf(parentStr = params.nodes[0].toString().substr(0, params.nodes[0].toString().lastIndexOf(".")))) != -1) {
                for (var i = openedCluster.length - 1; i >= index; --i) {
                    if (openedCluster[i].includes(parentStr)) {
                        clusterToClose.push((function () {
                            for (clusterType of window.clusterTypes[context]) {
                                if (clusterType[0] == openedCluster[i])
                                    return clusterType;
                            }
                        })());
                        openedCluster.splice(i, 1); // Remove clustered from array
                    }
                }
                clusterByModule(clusterToClose); // Cluster
            }
        }
    });

    function clusterByModule(clusterTypes) {
        for (var clusterType of clusterTypes) {
            var clusterOptionsByData = {
                joinCondition: function(childOptions) {
                    return (childOptions.id.endsWith(clusterType[0].substr(2) + '.', clusterType[0].length + 1));
                },
                processProperties: function (clusterOptions, childNodes, childEdges) {
                    var titleArr = [];
                    for (var i = 0; i < childNodes.length; i++) {
                        if (typeof childNodes[i].title != 'undefined') {
                            clusterOptions.color = 'red';
                            for (var item of childNodes[i].title.split(' / ')) {
                                if (titleArr.indexOf(item) == -1) {
                                    titleArr.push(item);
                                }
                            }
                        }
                    }
                    if (titleArr.length != 0) {
                        clusterOptions.title = titleArr.join(' / ');
                    }
                    return clusterOptions;
                },
                clusterNodeProperties: {
                    id: clusterType[0],
                    label : clusterType[0],
                    color : clusterColor[((clusterType[1] >= 4) ? 4 : clusterType[1]) - 1],
                    allowSingleNodeCluster: true
                }
            };
            network.cluster(clusterOptionsByData);
        }
    }

    function applyMapping(nodeMapped) {
        nodes[context].update(nodeMapped);
    }
}


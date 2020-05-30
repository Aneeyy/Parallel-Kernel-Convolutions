const {remote,dialog,app} = require('electron').remote;
const fs = require('fs');
const path = require("path");

let kernel = null;

let transferDataDirectory = app.getAppPath()
transferDataDirectory = transferDataDirectory.substr(0,transferDataDirectory.length-4)
transferDataDirectory += "/transferData"

function loadJSON(fileName, cb){
    fs.readFile(path.join(transferDataDirectory,fileName), (err, data) => {
        if (err) throw err;
        cb(JSON.parse(data));
    });
}

function writeJSON(fileName, object){
    fs.writeFileSync(path.join(transferDataDirectory,fileName),  JSON.stringify(object));
}

function setConfigInputFile(path){
    loadJSON("config.json", (data)=>{
        data.fileInputLocation = path;
        writeJSON("config.json",data);
    });
}

function copyImageFile(imagePath){
    let newImagePath = path.join(transferDataDirectory,path.basename(imagePath));
    console.log("new Image path: ",newImagePath);

    fs.copyFile(imagePath, newImagePath, (err) => {
        if (err) throw err;

        setConfigInputFile(newImagePath);

        // document.getElementById("leftContainerInstruction").style.display = "none";
        let leftImageContainer = document.getElementById("leftImageContainer")
        leftImageContainer.innerHTML = "<img src='" + newImagePath + "'>"

        leftImageContainer.style.display = "flex";

    });
}

function openFileSelectDialog(){
    dialog.showOpenDialog({
        properties: ['openFile'],
        filters: [
            { name: 'Images', extensions: ['jpg', 'png'] },
        ]
    }).then(result => {
        if(result.canceled){
            console.log("User did not select a file");
            return;
        }
        let imagePath = result.filePaths[0];
        copyImageFile(imagePath);

       //console.log(transferDataDirectory)
    }).catch(err => {
        console.log(err)
    })
}

function clearOldImageFiles(){
    fs.readdir(transferDataDirectory, (err, files) => {
        if (err) throw err;

        for (const file of files) {
            if(file === "config.json" || file === "OMPTiming.json"|| file === "pythonTiming.json"){

                continue;
            }
            fs.unlink(path.join(transferDataDirectory, file), err => {
                if (err) throw err;
            });
        }
    });
}
function clearOldTimingData(){
    let td = {"timing":null,fileOutputLocation:null};

    writeJSON("OMPTiming.json",td);
    writeJSON("pythonTiming.json",td);
}
//if no old kernels were used, this will fail
function loadConfigAndClearImageData(){
    let config;
    loadJSON("config.json", (data)=>{
        config = data;

        kernel = config.kernel;
       // config.kernel = kernel;
        config.fileInputLocation = null;
        writeJSON("config.json",config);
    });


    return kernel;

}

function loadDefaultKernel(){
    kernel = [
        [0,0,0],
        [0,1,0],
        [0,0,0]
    ]
}



function main(){
    clearOldImageFiles();
    clearOldTimingData();
    loadConfigAndClearImageData((kernel)=>{
        if(!kernel){
            loadDefaultKernel();
        }
    })



}

main();

let lc = document.getElementById("LeftContainer");
let lci = document.getElementById("leftContainerInstruction");
let dragMask = document.getElementById("dragMask");
// lci.addEventListener('dragleave', (event)=>{
//     event.preventDefault();
// })

lc.addEventListener('dragenter', (event)=>{
    // lc.classList.add("highlight")
    lc.classList.add("highlight")

    dragMask.style.display="flex";
})
// dragMask.addEventListener('dragenter', (event)=>{
//     lc.classList.add("highlight")
//     // dragMask.style.display="flex";
// })
dragMask.addEventListener('dragleave', (event)=>{
    lc.classList.remove("highlight")
    dragMask.style.display="none";
})
dragMask.addEventListener('drop', (event)=>{
    lc.classList.remove("highlight")
    dragMask.style.display="none";
    let files = event.dataTransfer.files;

    if(files.length === 1){
        console.log(files[0].path);
        copyImageFile(files[0].path);
    }


})


dragMask.addEventListener('dragenter', preventDefaults, false)
dragMask.addEventListener('dragover', preventDefaults, false)
dragMask.addEventListener('dragleave', preventDefaults, false)
dragMask.addEventListener('drop', preventDefaults, false)

function preventDefaults (e) {
  e.preventDefault()
  e.stopPropagation()
}

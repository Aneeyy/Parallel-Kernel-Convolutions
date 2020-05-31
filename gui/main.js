const {remote,dialog,app} = require('electron').remote;
const fs = require('fs');
const path = require("path");
const { spawn } = require('child_process');

const pythonScript = "../python/kernelConvolution.py"
const openMPExecutable = "../OpenMP/openMP"

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

function setConfigKernel(){
    loadJSON("config.json", (data)=>{
        data.kernel = kernel;
        writeJSON("config.json",data);
    });
}

function setConfigInputFile(path,openMPPath,pythonPath){
    loadJSON("config.json", (data)=>{
        data.fileInputLocation = path;
        data.openMPOutputLocation = openMPPath;
        data.pythonOutputLocation = pythonPath;
        writeJSON("config.json",data);
    });
}

function copyImageFile(imagePath){
    clearOldImageFiles();
    let newImagePath = path.join(transferDataDirectory,path.basename(imagePath));
    let openMPPath = path.join(transferDataDirectory,"OMP-"+path.basename(imagePath));
    let pythonPath = path.join(transferDataDirectory,"python-"+path.basename(imagePath));
    console.log("new Image path: ",newImagePath);

    fs.copyFile(imagePath, newImagePath, (err) => {
        if (err) throw err;

        setConfigInputFile(newImagePath,openMPPath,pythonPath);

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

    document.getElementById("OMPTiming").innerHTML = "";
    document.getElementById("pythonTiming").innerHTML = "";
    document.getElementById("resultsDiff").innerHTML = "";
    document.getElementById("usingImage").innerHTML = "";

    writeJSON("OMPTiming.json",td);
    writeJSON("pythonTiming.json",td);
}
//if no old kernels were used, this will fail
function loadConfigAndClearImageData(){
    let config;
    loadJSON("config.json", (data)=>{
        config = data;
        if(validKernel(config.kernel)){
            kernel = config.kernel;
        }
        else{
            loadDefaultKernel();
        }
        syncKernelToGUI();


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
    syncKernelToGUI();
}
function createBlankKernel(size){
    let kernel = []
    for(let r = 0; r < size; r++){
        let row = [];
        for(let c = 0; c < size; c++){
            row.push(0);
        }
        kernel.push(row);
    }
    return kernel;

}
function validKernel(k){
    if(!k){
        return k;
    }
    let size = k.length;
    if(size !== 3 || size !== 5){
        return false;
    }

    for(let r = 0; r < size; r++){
        if(k[r].length !== size){
            return false;
        }
        for(let c = 0; c < size; c++){
            if(!Number.isInteger(k[r][c])){
                return false;
            }
        }
    }

    return true;
}


function syncKernelToGUI(){

    let ks = "k";
    if(kernel.length === 3) {
        ks += "3";
    }
    else if(kernel.length === 5) {
        ks += "5";
    }

    for(let r = 0; r < kernel.length; r++){
        for(let c = 0; c< kernel.length; c++){
            let kId = ks + "row" + (r+1) + "col" + (c+1);
            document.getElementById(kId).value = kernel[r][c];
        }
    }

}
function readKernelFromGUI(){
    let selector = document.getElementById("kernelSizeSelector");
    let ks = "k";
    if(selector.value === "3"){
        ks += "3";
    }
    else if(selector.value === "5"){
        ks += "5";
    }
    let n = parseInt(selector.value);
    kernel = createBlankKernel(n);
    for(let r = 0; r < kernel.length; r++){
        for(let c = 0; c< kernel.length; c++){
            let kId = ks + "row" + (r+1) + "col" + (c+1);
             kernel[r][c] = document.getElementById(kId).value;
        }
    }

}

document.querySelectorAll('.kInput').forEach(item => {
    item.addEventListener('change', (event) => {
        readKernelFromGUI();
        console.log(kernel);
    })
})


function changeKernelSize(){
    let selector = document.getElementById("kernelSizeSelector");
    if(selector.value === "3"){
        if(kernel.length !== 3){
            let newKernel = createBlankKernel(3);
            for(let r = 1; r < 4; r++){
                for(let c = 1; c < 4; c++){
                    newKernel[r-1][c-1] = kernel[r][c];
                }
            }
            kernel = newKernel;
            syncKernelToGUI();
        }
        document.getElementById("fiveKernel").style.display = "none";
        document.getElementById("threeKernel").style.display = "flex";


    }
    else{
        if(kernel.length !== 5){
            let newKernel = createBlankKernel(5);
            for(let r = 0; r < 3; r++){
                for(let c = 0; c < 3; c++){
                    newKernel[r+1][c+1] = kernel[r][c];
                }
            }
            kernel = newKernel;
            syncKernelToGUI();
        }
        document.getElementById("threeKernel").style.display = "none";
        document.getElementById("fiveKernel").style.display = "flex";

    }
}

function clearResultImage(){
    document.getElementById("parameters").style.display = "flex";
    document.getElementById("imageResults").style.display = "none";
}
function displayResults(OMPTiming,pythonTiming){
    let fastestTiming = OMPTiming;
    let using = "OMP"
    if(pythonTiming.timing < OMPTiming.timing){
        fastestTiming = pythonTiming;
        using = "Python"
    }

    let rightImageContainer = document.getElementById("rightImageContainer")
    rightImageContainer.innerHTML = "<img src='" + fastestTiming.fileOutputLocation + "'>"
    rightImageContainer.style.display = "flex";

    document.getElementById("parameters").style.display = "none";
    document.getElementById("imageResults").style.display = "flex";

    document.getElementById("OMPTiming").innerHTML = "OMP Time: " + OMPTiming.timing + "s";
    document.getElementById("pythonTiming").innerHTML = "Python Time: " + pythonTiming.timing + "s";
    //document.getElementById("resultsDiff").innerHTML = "Results Diff: "
    document.getElementById("usingImage").innerHTML = "Image From: " + using;

    //const pythonChild = spawn("diff",[pythonScript]);



}

function getAndDisplayResults(){
    let OMPTiming, pythonTiming;

    loadJSON("../transferData/OMPTiming.json", (t)=>{
        OMPTiming = t;
        if(pythonTiming){
            displayResults(OMPTiming,pythonTiming);
        }
    })
    loadJSON("../transferData/pythonTiming.json", (t)=>{
        pythonTiming = t;
        if(OMPTiming){
            displayResults(OMPTiming,pythonTiming);
        }
    })

}

function performFilter(){
    setConfigKernel();
    clearOldTimingData();

    const pythonChild = spawn("python",[pythonScript]);
    const openMPChild = spawn(openMPExecutable);
    let pythonDone = false;
    let openMPDone = false;

    pythonChild.stdout.on('data', (data) => {
        console.log(`python stdout: ${data}`);
    });
    pythonChild.stderr.on('data', (data) => {
        console.log(`python stderr: ${data}`);
    });
    openMPChild.stdout.on('data', (data) => {
        console.log(`openMP stdout: ${data}`);
    });
    openMPChild.stderr.on('data', (data) => {
        console.log(`openMP stderr: ${data}`);
    });


    pythonChild.on('close', (code) => {
        console.log(`Python process exited with code ${code}`);
        pythonDone = true;
        if(openMPDone && pythonDone){
            getAndDisplayResults();
        }
    });

    openMPChild.on('close', (code) => {
        console.log(`openMP process exited with code ${code}`);
        openMPDone = true;
        if(openMPDone && pythonDone){
            getAndDisplayResults();
        }
    });


}


function main(){
    clearOldImageFiles();
    clearOldTimingData();
    loadConfigAndClearImageData()



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

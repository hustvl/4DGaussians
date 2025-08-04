from fastapi import FastAPI, Request
from pydantic import BaseModel
import subprocess
import os

app = FastAPI()


class JobRequest(BaseModel):
    scene_path: str
    config: str
    expname: str
    checkpoint: str = None


@app.post("/start-job")
def start_job(job: JobRequest):
    output_dir = os.path.join("output", job.expname)
    os.makedirs(output_dir, exist_ok=True)

    cmd = [
        "python",
        "run_training_job.py",
        "--scene_path",
        job.scene_path,
        "--config",
        job.config,
        "--expname",
        job.expname,
        "--output_dir",
        output_dir,
    ]
    if job.checkpoint:
        cmd += ["--checkpoint", job.checkpoint]

    print(f"Launching job: {' '.join(cmd)}")
    subprocess.Popen(cmd)

    return {"status": "started", "expname": job.expname}

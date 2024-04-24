import requests
import os
import sys
import shutil
from dotenv import load_dotenv

load_dotenv()
access_key = os.getenv("UNSPLASH_ACCESS_KEY")

def main():
    if len(sys.argv) != 2:
        print("Usage: python generateImages.py <num>")
        sys.exit(1)

    num = sys.argv[1]

    save_dir = "images"
    if os.path.exists(save_dir):
        shutil.rmtree(save_dir)
    os.makedirs(save_dir)

    for i in range(int(num)):
        url = f"https://api.unsplash.com/photos/random?client_id={access_key}"
        response = requests.get(url)
        if response.status_code == 200:
            image_url = response.json()["urls"]["regular"]
            image_data = requests.get(image_url).content
            with open(os.path.join(save_dir, f"image_{i+1}.jpg"), "wb") as f:
                f.write(image_data)
        else:
            print(f"Failed to fetch image {i+1}")


if __name__ == "__main__":
    main()

FROM node:14.16.1

WORKDIR /app

ENV CLOE_UI_SERVER_ROOT_DIR /app

# Install dependencies
COPY package.json ./
COPY yarn.lock ./
RUN yarn install

COPY *.js ./

EXPOSE 4000

# Start Server
ENTRYPOINT ["npm", "run", "server"]
